"""현재 플레이어 스켈레톤에 맞는 BombGun 발사·후방 점프·착지 자세를 생성한다."""
import importlib.util
from pathlib import Path
import unreal

source = Path(__file__).with_name('create_sorceress_motions.py')
spec = importlib.util.spec_from_file_location('bomb_pose_tools', source)
motion = importlib.util.module_from_spec(spec)
spec.loader.exec_module(motion)
motion.OUTPUT = '/Game/PlayerCharacter/Animations/SorceressSmooth'
motion.FPS = 60
idle_pose = motion.make_pose


def smooth(t):
    t = max(0.0, min(1.0, t))
    return t*t*t*(t*(t*6-15)+10)


def make_pose(kind, t, duration):
    local = idle_pose('Idle', 0, 2.4)
    phase = t/duration
    strength = motion.envelope(phase, [(0, 0), (0.2, 1), (0.55, 0.8), (1, 0)])
    p, q = local['pelvis']
    local['pelvis'] = (motion.add(p, (0, -strength, -2*strength)), q)
    for bone in ('spine_01', 'spine_03', 'spine_05'):
        motion.turn(local, bone, (1, 0, 0), 10*strength/3)
    for side, sign in (('l', 1), ('r', -1)):
        # 캡슐의 후퇴·접지는 C++에 맡기고 발과 무릎은 작은 점프의 자세만 표현한다.
        foot = motion.add(motion.WORLD['foot_'+side][0], (0, -2*strength, 3*strength))
        motion.solve_limb(local, ('thigh_'+side, 'calf_'+side, 'foot_'+side), foot, (0, 1, 0), True)
        idle_hand = (sign*20, 10, 69)
        firing_hand = (5, 25, 80) if side == 'l' else (-5, 16, 80)
        hand = motion.mix(idle_hand, firing_hand, strength)
        motion.solve_limb(local, ('upperarm_'+side, 'lowerarm_'+side, 'hand_'+side), hand, (sign, -0.3, -0.4))
    # GunProp의 로컬 -X 총열을 캐릭터 전방으로 들어 발사하는 자세를 만든다.
    rotation = motion.components(local)['hand_r'][1]
    barrel = motion.rotate(rotation, (-1, 0, 0))
    delta = motion.between(barrel, (0, 1, 0.05))
    weighted = motion.norm(motion.mix((0, 0, 0, 1), delta, strength))
    motion.set_world_rotation(local, 'hand_r', motion.qm(weighted, rotation))
    return local


def main():
    assert not unreal.EditorLevelLibrary.get_pie_worlds(False), 'PIE 종료 후 실행하세요.'
    path = motion.OUTPUT+'/AS_Sorceress_BombRecoil'
    assert not unreal.EditorAssetLibrary.does_asset_exist(path), '기존 애셋은 덮어쓰지 않습니다: '+path
    motion.smooth = smooth
    motion.make_pose = make_pose
    clip, poses = motion.create_sequence('BombRecoil', 0.3)
    first, last = motion.components(poses[0]), motion.components(poses[-1])
    for bone in ('pelvis', 'hand_l', 'hand_r', 'foot_l', 'foot_r'):
        assert motion.length(motion.sub(first[bone][0], last[bone][0])) < 0.001
    bp = unreal.load_asset('/Game/Blueprints/Player/BP_LostArkCharacter')
    unreal.BlueprintEditorLibrary.compile_blueprint(bp)
    bp.modify()
    character = unreal.get_default_object(bp.generated_class())
    character.modify()
    character.set_editor_property('bomb_recoil_animation', clip)
    # 스폰 시 사용하는 Blueprint 기본값 초기화 목록도 새 애니메이션 참조로 갱신한다.
    unreal.BlueprintEditorLibrary.compile_blueprint(bp)
    assert unreal.EditorAssetLibrary.save_loaded_asset(bp, only_if_is_dirty=False)
    unreal.log('BOMB_RECOIL_MOTION_CONNECTED '+clip.get_path_name())


if __name__ == '__main__':
    main()
