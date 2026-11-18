"""기존 원본을 보존하며 부드러운 모션 변형과 시작 일어서기를 생성·연결한다."""
import importlib.util
import json
import math
from pathlib import Path
import unreal

source = Path(__file__).with_name('create_sorceress_motions.py')
spec = importlib.util.spec_from_file_location('original_motions', source)
motion = importlib.util.module_from_spec(spec)
spec.loader.exec_module(motion)
motion.OUTPUT = '/Game/PlayerCharacter/Animations/SorceressSmooth'
motion.FPS = 60
original_pose = motion.make_pose


def smoother(t):
    # 양 끝의 속도와 가속도가 0인 곡선으로 방향 전환의 튐을 줄인다.
    t = max(0.0, min(1.0, t))
    return t*t*t*(t*(t*6-15)+10)


def make_pose(kind, t, duration):
    local = original_pose('Idle' if kind == 'Startup' else kind, t, duration)
    if kind == 'Walk':
        # 발이 지면에 닿는 순간 높이 변화율을 0으로 만들어 착지 충격을 줄인다.
        phase = 2*math.pi*t/duration
        for side, sign in (('l', 1), ('r', -1)):
            lift = max(0, sign*math.cos(phase))
            target = motion.add(motion.WORLD['foot_'+side][0], (0, 13*sign*math.sin(phase), 6*lift*lift))
            motion.solve_limb(local, ('thigh_'+side, 'calf_'+side, 'foot_'+side), target, (0, 1, 0), True)
    elif kind == 'Startup':
        # 한 손을 바닥에 짚고 한쪽 무릎을 깊게 굽힌 뒤 골반·상체·머리 순으로 일어난다.
        crouch = 1-smoother((t-0.35)/1.65)
        chest = 1-smoother((t-0.55)/1.65)
        hand = 1-smoother((t-0.45)/1.35)
        local['pelvis'] = (motion.add(motion.LOCAL['pelvis'][0], (-3*crouch, -5*crouch, -0.6-48*crouch)), motion.LOCAL['pelvis'][1])
        for bone in ('spine_01', 'spine_03', 'spine_05'):
            motion.turn(local, bone, (1, 0, 0), -95*chest/3)
        motion.turn(local, 'head', (1, 0, 0), -12*(1-smoother((t-0.8)/1.4)))
        for side, sign in (('l', 1), ('r', -1)):
            foot = motion.add(motion.WORLD['foot_'+side][0], (sign*3*crouch, (18 if side == 'r' else -17)*crouch, 0))
            # 뒤쪽 무릎은 지면 위에 두고, 일어날 때 기존 보행 무릎 방향으로 복귀한다.
            pole = (0, 1, 0.28*crouch if side == 'l' else 0)
            motion.solve_limb(local, ('thigh_'+side, 'calf_'+side, 'foot_'+side), foot, pole, True)
            target = motion.mix((sign*20, 10, 69), (12, 26, 7) if side == 'l' else (-19, 21, 39), hand)
            motion.solve_limb(local, ('upperarm_'+side, 'lowerarm_'+side, 'hand_'+side), target, (sign, -0.3, -0.4))
    return local


def main():
    motion.smooth = smoother
    motion.make_pose = make_pose
    durations = {'Idle': 2.4, 'Walk': 1.0, 'Slide': 0.4, 'Hit': 0.4, 'FallDown': 2.3, 'Fireball': 0.4, 'Startup': 2.6}
    for kind in durations:
        for prefix in ('AS_', 'AM_'):
            path = motion.OUTPUT+'/'+prefix+'Sorceress_'+kind
            if unreal.EditorAssetLibrary.does_asset_exist(path):
                raise RuntimeError('기존 결과 보존: '+path)
    clips, montages, report = {}, {}, {}
    for kind, duration in durations.items():
        clips[kind], poses = motion.create_sequence(kind, duration)
        report[kind] = [{b: motion.components(p)[b][0] for b in ('pelvis', 'head', 'hand_l', 'foot_l', 'foot_r')} for p in poses]
        if kind not in ('Idle', 'Walk', 'Startup'):
            montages[kind] = motion.create_montage(kind, clips[kind])
            for key in ('blend_in', 'blend_out'):
                blend = montages[kind].get_editor_property(key)
                blend.set_editor_property('blend_option', unreal.AlphaBlendOption.QUINTIC_IN_OUT)
                montages[kind].set_editor_property(key, blend)
            unreal.EditorAssetLibrary.save_loaded_asset(montages[kind])
    bp = unreal.load_asset('/Game/Blueprints/Player/BP_LostArkCharacter')
    cdo = unreal.get_default_object(bp.generated_class())
    for prop, kind in (('idle_animation', 'Idle'), ('walk_animation', 'Walk'), ('startup_animation', 'Startup')):
        cdo.set_editor_property(prop, clips[kind])
    for prop, kind in (('slide_montage', 'Slide'), ('hit_montage', 'Hit'), ('fall_down_montage', 'FallDown'), ('attack_montage1', 'Fireball'), ('attack_montage2', 'Fireball')):
        cdo.set_editor_property(prop, montages[kind])
    # BombGun의 원본 트랙·Notify·재생 담당은 그대로 두고 복제본의 블렌딩만 조정한다.
    bomb = cdo.get_editor_property('bomb_montage')
    if bomb:
        bomb = unreal.EditorAssetLibrary.duplicate_asset(bomb.get_path_name(), motion.OUTPUT+'/AM_Player_BombSmooth')
        for key in ('blend_in', 'blend_out'):
            blend = bomb.get_editor_property(key)
            blend.set_editor_property('blend_option', unreal.AlphaBlendOption.QUINTIC_IN_OUT)
            bomb.set_editor_property(key, blend)
        unreal.EditorAssetLibrary.save_loaded_asset(bomb)
        cdo.set_editor_property('bomb_montage', bomb)
    unreal.BlueprintEditorLibrary.compile_blueprint(bp)
    unreal.EditorAssetLibrary.save_loaded_asset(bp)
    path = Path(unreal.Paths.project_saved_dir())/'MotionReference/smooth_poses.json'
    path.write_text(json.dumps(report), encoding='utf-8')
    unreal.log('SMOOTH_STARTUP_SETUP_COMPLETE')


if __name__ == '__main__':
    main()
