"""생성된 애셋을 다시 로드하여 호환성, 반복 경계, Montage 연결을 검증한다."""
import json
import math
from pathlib import Path
import unreal


def main():
    folder = '/Game/PlayerCharacter/Animations/Sorceress/'
    mesh = unreal.load_asset('/Game/FinalCharacter/PlayerCharacter_Final')
    skeleton = mesh.get_editor_property('skeleton')
    expected = {'Idle': 2.4, 'Walk': 1.0, 'Slide': 0.4, 'Hit': 0.4, 'FallDown': 2.3, 'Fireball': 0.4}
    report = {}
    for kind, duration in expected.items():
        animation = unreal.load_asset(folder+'AS_Sorceress_'+kind)
        assert animation is not None, kind+' 애셋 없음'
        assert animation.get_editor_property('skeleton') == skeleton, kind+' Skeleton 불일치'
        assert not animation.get_editor_property('enable_root_motion'), kind+' 루트 모션 중복 위험'
        actual_duration = animation.get_editor_property('sequence_length')
        assert abs(actual_duration-duration) < 0.001, kind+' 길이 불일치'
        poses = [unreal.AnimPoseExtensions.get_anim_pose_at_time(animation, t, unreal.AnimPoseEvaluationOptions())
                 for t in (0, duration/2, duration)]
        compressed_options = unreal.AnimPoseEvaluationOptions()
        compressed_options.set_editor_property('evaluation_type', unreal.AnimDataEvalType.COMPRESSED)
        compressed = [unreal.AnimPoseExtensions.get_anim_pose_at_time(animation, t, compressed_options)
                      for t in (0, duration/2, duration)]
        for raw_pose, runtime_pose in zip(poses, compressed):
            for b in ('root', 'pelvis', 'head', 'hand_l', 'hand_r', 'foot_l', 'foot_r'):
                raw = unreal.AnimPoseExtensions.get_bone_pose(raw_pose, b, unreal.AnimPoseSpaces.WORLD)
                packed = unreal.AnimPoseExtensions.get_bone_pose(runtime_pose, b, unreal.AnimPoseSpaces.WORLD)
                assert (raw.translation-packed.translation).length() < 0.5, kind+' 압축 후 자세 오차: '+b
        for pose in poses:
            for b in ('root', 'pelvis', 'head', 'hand_l', 'hand_r', 'foot_l', 'foot_r'):
                tr = unreal.AnimPoseExtensions.get_bone_pose(pose, b, unreal.AnimPoseSpaces.WORLD)
                assert all(math.isfinite(v) for v in (tr.translation.x, tr.translation.y, tr.translation.z,
                                                     tr.rotation.x, tr.rotation.y, tr.rotation.z, tr.rotation.w))
        if kind in ('Idle', 'Walk'):
            for b in ('root', 'pelvis', 'head', 'hand_l', 'hand_r', 'foot_l', 'foot_r'):
                first = unreal.AnimPoseExtensions.get_bone_pose(poses[0], b, unreal.AnimPoseSpaces.WORLD)
                last = unreal.AnimPoseExtensions.get_bone_pose(poses[-1], b, unreal.AnimPoseSpaces.WORLD)
                assert (first.translation-last.translation).length() < 0.1, kind+' 반복 경계 위치 불연속: '+b
        if kind not in ('Idle', 'Walk'):
            montage = unreal.load_asset(folder+'AM_Sorceress_'+kind)
            assert montage.get_editor_property('skeleton') == skeleton
            tracks = montage.get_editor_property('slot_anim_tracks')
            assert len(tracks) == 1 and str(tracks[0].get_editor_property('slot_name')) == 'DefaultSlot'
            assert abs(montage.get_editor_property('sequence_length')-duration) < 0.001
        report[kind] = {'seconds': actual_duration, 'valid': True}
    blueprint = unreal.load_asset('/Game/Blueprints/Player/BP_LostArkCharacter')
    cdo = unreal.get_default_object(blueprint.generated_class())
    anim_class = cdo.get_editor_property('mesh').get_editor_property('anim_class')
    assert anim_class.get_path_name() == '/Script/LostArk.SorceressAnimInstance'
    for prop, kind, prefix in [('idle_animation', 'Idle', 'AS'), ('walk_animation', 'Walk', 'AS'),
                               ('slide_montage', 'Slide', 'AM'), ('hit_montage', 'Hit', 'AM'),
                               ('fall_down_montage', 'FallDown', 'AM'),
                               ('attack_montage1', 'Fireball', 'AM'), ('attack_montage2', 'Fireball', 'AM')]:
        assert cdo.get_editor_property(prop) == unreal.load_asset(folder+prefix+'_Sorceress_'+kind), prop+' 연결 불일치'
    path = Path(unreal.Paths.project_saved_dir())/'MotionReference/validation.json'
    path.write_text(json.dumps(report, indent=2), encoding='utf-8')
    unreal.log('MOTION_VALIDATION_PASS '+str(path))


if __name__ == '__main__':
    main()
