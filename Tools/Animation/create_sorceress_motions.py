"""현재 플레이어의 실제 기준 포즈에서 원본 소서리스풍 모션 초안을 만든다.

Unreal Python commandlet용. 기존 애셋을 덮어쓰지 않으며, 생성이 모두 끝난 뒤
플레이어 Blueprint의 애니메이션 참조만 연결한다. 게임 규칙/데미지/입력은 유지한다.
"""
import json
import math
from pathlib import Path
import unreal

OUTPUT = '/Game/PlayerCharacter/Animations/Sorceress'
FPS = 30
MESH = unreal.load_asset('/Game/FinalCharacter/PlayerCharacter_Final')
SKELETON = MESH.get_editor_property('skeleton')
POSE = unreal.AnimPoseExtensions.get_reference_pose(SKELETON)
BONES = [str(b) for b in unreal.AnimPoseExtensions.get_bone_names(POSE)]
PARENTS = {b: str(MESH.get_bone_parent(b)) for b in BONES}
TOOLS = unreal.AssetToolsHelpers.get_asset_tools()


# 벡터/쿼터니언 계산은 표준 Python만 사용하여 추가 의존성을 만들지 않는다.
def add(a, b): return tuple(x+y for x, y in zip(a, b))
def sub(a, b): return tuple(x-y for x, y in zip(a, b))
def mul(a, s): return tuple(x*s for x in a)
def dot(a, b): return sum(x*y for x, y in zip(a, b))
def length(a): return math.sqrt(dot(a, a))
def norm(a): return mul(a, 1.0 / max(length(a), 1.e-8))
def cross(a, b): return (a[1]*b[2]-a[2]*b[1], a[2]*b[0]-a[0]*b[2], a[0]*b[1]-a[1]*b[0])
def mix(a, b, t): return add(mul(a, 1-t), mul(b, t))
def inv(q): return (-q[0], -q[1], -q[2], q[3])


def qm(a, b):
    v = add(add(mul(b[:3], a[3]), mul(a[:3], b[3])), cross(a[:3], b[:3]))
    return (*v, a[3]*b[3]-dot(a[:3], b[:3]))


def rotate(q, v): return qm(qm(q, (*v, 0)), inv(q))[:3]


def axis_angle(axis, degrees):
    h = math.radians(degrees) * 0.5
    return (*mul(norm(axis), math.sin(h)), math.cos(h))


def between(a, b):
    a, b = norm(a), norm(b)
    d = max(-1.0, min(1.0, dot(a, b)))
    if d < -0.99999:
        return axis_angle(cross(a, (0, 0, 1) if abs(a[2]) < 0.9 else (1, 0, 0)), 180)
    return norm((*cross(a, b), 1+d))


def smooth(t):
    t = max(0.0, min(1.0, t))
    return t*t*(3-2*t)


def envelope(t, points):
    for (ta, va), (tb, vb) in zip(points, points[1:]):
        if t <= tb:
            u = smooth((t-ta)/(tb-ta))
            return va+(vb-va)*u
    return points[-1][1]


LOCAL, WORLD = {}, {}
for bone in BONES:
    for table, space in ((LOCAL, unreal.AnimPoseSpaces.LOCAL), (WORLD, unreal.AnimPoseSpaces.WORLD)):
        tr = unreal.AnimPoseExtensions.get_ref_bone_pose(POSE, bone, space)
        p, q = tr.translation, tr.rotation
        table[bone] = ((p.x, p.y, p.z), (q.x, q.y, q.z, q.w))


def components(local):
    result = {}
    for b in BONES:
        p, q = local[b]
        parent = PARENTS[b]
        if parent in result:
            pp, pq = result[parent]
            result[b] = (add(pp, rotate(pq, p)), qm(pq, q))
        else:
            result[b] = (p, q)
    return result


def set_world_rotation(local, bone, rotation):
    comp = components(local)
    parent = PARENTS[bone]
    pq = comp[parent][1] if parent in comp else (0, 0, 0, 1)
    local[bone] = (local[bone][0], qm(inv(pq), rotation))


def turn(local, bone, axis, angle):
    # 기준 포즈의 축 방향 차이를 보정하여 컴포넌트 공간 기준으로 회전한다.
    current = components(local)[bone][1]
    set_world_rotation(local, bone, qm(axis_angle(axis, angle), current))


def solve_limb(local, chain, target, pole, keep_foot=False):
    a, b, c = chain
    comp = components(local)
    start = comp[a][0]
    l1, l2 = length(LOCAL[b][0]), length(LOCAL[c][0])
    d = max(abs(l1-l2)+0.01, min(length(sub(target, start)), l1+l2-0.01))
    direction = norm(sub(target, start))
    end = add(start, mul(direction, d))
    side = norm(sub(pole, mul(direction, dot(pole, direction))))
    along = (l1*l1-l2*l2+d*d)/(2*d)
    knee = add(add(start, mul(direction, along)), mul(side, math.sqrt(max(0, l1*l1-along*along))))
    for bone, child, wanted in ((a, b, knee), (b, c, end)):
        comp = components(local)
        old = sub(comp[child][0], comp[bone][0])
        new = sub(wanted, comp[bone][0])
        set_world_rotation(local, bone, qm(between(old, new), comp[bone][1]))
    if keep_foot:
        set_world_rotation(local, c, WORLD[c][1])


def make_pose(kind, t, duration):
    local = dict(LOCAL)
    breath = math.sin(2*math.pi*t/2.4)
    pelvis = (0.0, 0.0, -0.6+0.35*breath)
    lean, twist, fall = -1.5, 0.0, 0.0
    hands = {'l': (20, 10, 69+0.5*breath), 'r': (-20, 10, 69+0.5*breath)}
    feet = {s: WORLD['foot_'+s][0] for s in ('l', 'r')}
    if kind == 'Walk':
        phase = 2*math.pi*t/duration
        pelvis = (0.7*math.sin(phase), 0, -2+0.7*math.cos(2*phase))
        lean, twist = -4, 2.5*math.sin(phase)
        for side, sign in (('l', 1), ('r', -1)):
            s = sign*math.sin(phase)
            feet[side] = add(feet[side], (0, 13*s, 6*max(0, sign*math.cos(phase))))
            hands[side] = (sign*20, 10-7*s, 68+1.2*max(0, s))
    elif kind == 'Slide':
        crouch = envelope(t, [(0, 0), (0.09, 1), (0.27, 1), (duration, 0)])
        pelvis, lean = (0, -3*crouch, -16*crouch), -22*crouch
        feet['l'] = add(feet['l'], (1*crouch, 14*crouch, 0))
        feet['r'] = add(feet['r'], (-1*crouch, -14*crouch, 0))
        hands = {'l': mix(hands['l'], (24, -9, 64), crouch), 'r': mix(hands['r'], (-24, -12, 65), crouch)}
    elif kind == 'Hit':
        recoil = envelope(t, [(0, 0), (0.10, 1), (0.22, 0.55), (duration, 0)])
        lean, twist = 12*recoil, -7*recoil
        pelvis = (0, -2*recoil, -2*recoil)
        for s, sign in (('l', 1), ('r', -1)):
            hands[s] = mix(hands[s], (sign*15, 14, 80), recoil)
    elif kind == 'FallDown':
        fall = envelope(t, [(0, 0), (0.42, 1), (0.90, 1), (1.35, 0.65), (1.85, 0.20), (duration, 0)])
        lean = -15*envelope(t, [(0, 0), (0.9, 0), (1.5, 1), (duration, 0)])
        pelvis = (0, 0, -5*fall)
        for s, sign in (('l', 1), ('r', -1)):
            hands[s] = mix(hands[s], (sign*21, 7, 74), fall)
            feet[s] = add(feet[s], (sign*2*fall, 9*fall, 6*fall))
    elif kind == 'Fireball':
        charge = envelope(t, [(0, 0), (0.22, 1), (0.30, 1), (duration, 0)])
        release = envelope(t, [(0, 0), (0.23, 0), (0.30, 1), (duration, 0)])
        # 두 손을 모아 준비한 다음 한 손을 전방으로 펴는 독자적인 시전 동작.
        hands['l'] = mix(hands['l'], (7, 21, 86), charge)
        hands['r'] = mix(hands['r'], (-7, 22, 86), charge)
        hands['r'] = mix(hands['r'], (-9, 26, 91), release)
        hands['l'] = mix(hands['l'], (15, 14, 83), release)
        lean, twist = 5*charge-10*release, 9*charge-18*release
    local['pelvis'] = (add(LOCAL['pelvis'][0], pelvis), LOCAL['pelvis'][1])
    for bone in ('spine_01', 'spine_03', 'spine_05'):
        turn(local, bone, (1, 0, 0), lean/3)
        turn(local, bone, (0, 0, 1), twist/3)
    for s, sign in (('l', 1), ('r', -1)):
        solve_limb(local, ('thigh_'+s, 'calf_'+s, 'foot_'+s), feet[s], (0, 1, 0), True)
        solve_limb(local, ('upperarm_'+s, 'lowerarm_'+s, 'hand_'+s), hands[s], (sign, -0.3, -0.4))
    # 손가락은 과도하게 펴지지 않도록 기존 기준 포즈에서 약하게 굽힌다.
    for b in BONES:
        if any(b.startswith(f+'_'+n+'_') for f in ('index', 'middle', 'ring', 'pinky') for n in ('01', '02', '03')):
            p, q = local[b]
            local[b] = (p, qm(q, axis_angle((0, 0, 1), 12)))
    if fall:
        # 캡슐 이동 없이 메시 안에서만 뒤로 넘어졌다가 일어나는 연출.
        q = axis_angle((1, 0, 0), 78*fall)
        center = components(local)['pelvis'][0]
        desired = add(center, (0, -8*fall, -42*fall))
        local['root'] = (sub(desired, rotate(q, center)), q)
    return local


def create_sequence(kind, duration):
    name = 'AS_Sorceress_'+kind
    path = OUTPUT+'/'+name
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        raise RuntimeError('기존 애셋 보존을 위해 중단: '+path)
    factory = unreal.AnimSequenceFactory()
    factory.set_editor_property('target_skeleton', SKELETON)
    factory.set_editor_property('preview_skeletal_mesh', MESH)
    asset = TOOLS.create_asset(name, OUTPUT, unreal.AnimSequence, factory)
    controller = asset.get_editor_property('controller')
    frames = round(duration*FPS)
    controller.open_bracket('소서리스 모션 초안 생성', False)
    controller.set_frame_rate(unreal.FrameRate(FPS, 1), False)
    controller.set_number_of_frames(unreal.FrameNumber(frames), False)
    poses = [make_pose(kind, i/FPS, duration) for i in range(frames+1)]
    for bone in BONES:
        controller.add_bone_curve(bone, False)
        positions = [unreal.Vector(*p[bone][0]) for p in poses]
        rotations = [unreal.Quat(*p[bone][1]) for p in poses]
        scales = [unreal.Vector(1, 1, 1) for _ in poses]
        if not controller.set_bone_track_keys(bone, positions, rotations, scales, False):
            raise RuntimeError('본 키 기록 실패: '+bone)
    controller.close_bracket(False)
    asset.set_editor_property('enable_root_motion', False)
    unreal.EditorAssetLibrary.save_loaded_asset(asset)
    unreal.log('MOTION_CREATED '+path)
    return asset, poses


def create_montage(kind, sequence):
    factory = unreal.AnimMontageFactory()
    factory.set_editor_property('source_animation', sequence)
    factory.set_editor_property('target_skeleton', SKELETON)
    montage = TOOLS.create_asset('AM_Sorceress_'+kind, OUTPUT, unreal.AnimMontage, factory)
    # 기존의 짧은 입력 잠금 안에서 자세가 이어지도록 블렌딩 시간만 조절한다.
    for key in ('blend_in', 'blend_out'):
        value = montage.get_editor_property(key)
        value.set_editor_property('blend_time', 0.05 if kind == 'Slide' else 0.08)
        montage.set_editor_property(key, value)
    unreal.EditorAssetLibrary.save_loaded_asset(montage)
    return montage


def main():
    durations = {'Idle': 2.4, 'Walk': 1.0, 'Slide': 0.4, 'Hit': 0.4, 'FallDown': 2.3, 'Fireball': 0.4}
    # 모든 경로를 먼저 검사하여 재실행 중 기존 결과가 덮어써지는 것을 방지한다.
    for kind in durations:
        if unreal.EditorAssetLibrary.does_asset_exist(OUTPUT+'/AS_Sorceress_'+kind):
            raise RuntimeError('이미 모션이 있습니다: '+kind)
        if kind not in ('Idle', 'Walk') and unreal.EditorAssetLibrary.does_asset_exist(OUTPUT+'/AM_Sorceress_'+kind):
            raise RuntimeError('이미 Montage가 있습니다: '+kind)
    clips, montages, preview = {}, {}, {}
    for kind, duration in durations.items():
        clips[kind], poses = create_sequence(kind, duration)
        preview[kind] = []
        for p in poses:
            comp = components(p)
            preview[kind].append({b: comp[b][0] for b in BONES})
        if kind not in ('Idle', 'Walk'):
            montages[kind] = create_montage(kind, clips[kind])
    output = Path(unreal.Paths.project_saved_dir())/'MotionReference'
    output.mkdir(parents=True, exist_ok=True)
    (output/'generated_poses.json').write_text(json.dumps({'parents': PARENTS, 'clips': preview}), encoding='utf-8')
    blueprint = unreal.load_asset('/Game/Blueprints/Player/BP_LostArkCharacter')
    cdo = unreal.get_default_object(blueprint.generated_class())
    cdo.set_editor_property('idle_animation', clips['Idle'])
    cdo.set_editor_property('walk_animation', clips['Walk'])
    cdo.set_editor_property('slide_montage', montages['Slide'])
    cdo.set_editor_property('hit_montage', montages['Hit'])
    cdo.set_editor_property('fall_down_montage', montages['FallDown'])
    cdo.set_editor_property('attack_montage1', montages['Fireball'])
    cdo.set_editor_property('attack_montage2', montages['Fireball'])
    mesh = cdo.get_editor_property('mesh')
    mesh.set_editor_property('animation_mode', unreal.AnimationMode.ANIMATION_BLUEPRINT)
    mesh.set_editor_property('anim_class', unreal.load_class(None, '/Script/LostArk.SorceressAnimInstance'))
    unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)
    unreal.EditorAssetLibrary.save_loaded_asset(blueprint)
    unreal.log('MOTION_SETUP_COMPLETE')


if __name__ == '__main__':
    main()
