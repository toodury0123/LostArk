"""현재 플레이어의 발바닥을 기존 캡슐 바닥에 맞추는 일회성 에디터 설정."""
import unreal


def main():
    # PIE 종료 요청은 비동기이므로 실제 월드가 모두 사라진 뒤에만 설정을 저장한다.
    assert not unreal.EditorLevelLibrary.get_pie_worlds(False), 'PIE를 완전히 종료한 뒤 실행하세요.'
    blueprint = unreal.load_asset('/Game/Blueprints/Player/BP_LostArkCharacter')
    character = unreal.get_default_object(blueprint.generated_class())
    mesh = character.get_editor_property('mesh')
    capsule = character.get_component_by_class(unreal.CapsuleComponent)
    rotation = mesh.get_editor_property('relative_rotation')
    assert abs(rotation.pitch) < 0.01 and abs(rotation.roll) < 0.01, '기울어진 메시에는 별도 보정이 필요합니다.'
    bounds = mesh.get_editor_property('skeletal_mesh_asset').get_imported_bounds()
    sole_min = bounds.origin.z-bounds.box_extent.z
    scale = mesh.get_editor_property('relative_scale3d').z
    old = mesh.get_editor_property('relative_location')
    # 기존 CharacterMovement가 평지에서 유지하는 캡슐-바닥 간격 2.15cm를 포함한다.
    target_z = -capsule.get_scaled_capsule_half_height()-2.15-sole_min*scale
    blueprint.modify()
    mesh.modify()
    mesh.set_editor_property('relative_location', unreal.Vector(old.x, old.y, target_z))
    unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)
    updated = unreal.get_default_object(blueprint.generated_class()).get_editor_property('mesh')
    assert abs(updated.get_editor_property('relative_location').z-target_z) < 0.01
    assert unreal.EditorAssetLibrary.save_loaded_asset(blueprint, only_if_is_dirty=False)
    unreal.log(f'PLAYER_FEET_ALIGNED mesh Z: {old.z:.3f} -> {target_z:.3f}; capsule unchanged')


if __name__ == '__main__':
    main()
