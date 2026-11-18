"""저장된 이전 PlayerStart 위치와 Z 보정 전 플레이어 메시 설정을 복구한다."""
import unreal


def main():
    assert not unreal.EditorLevelLibrary.get_pie_worlds(False), 'PIE를 완전히 종료한 뒤 실행하세요.'
    world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
    assert world and world.get_name() == 'TopDownMap', 'TopDownMap을 먼저 여세요.'
    starts = [a for a in unreal.GameplayStatics.get_all_actors_of_class(world, unreal.PlayerStart)
              if a.get_name() == 'PlayerStart_1']
    assert len(starts) == 1, '원래 PlayerStart를 유일하게 확인하지 못했습니다.'
    start = starts[0]
    # 별도 읽기 전용 프로세스에서 기존 디스크 맵의 위치를 확인한 값이다.
    original = unreal.Vector(1000.0, 1810.0, 1106.2458675782113)
    start.modify()
    start.set_actor_location(original, False, True)
    assert unreal.EditorLoadingAndSavingUtils.save_packages([start.get_outermost()], True)

    blueprint = unreal.load_asset('/Game/Blueprints/Player/BP_LostArkCharacter')
    mesh = unreal.get_default_object(blueprint.generated_class()).get_editor_property('mesh')
    old = mesh.get_editor_property('relative_location')
    blueprint.modify()
    mesh.modify()
    # 캡슐과 다른 설정은 유지하고, 이번 세션에서 변경했던 메시 Z만 되돌린다.
    mesh.set_editor_property('relative_location', unreal.Vector(old.x, old.y, -55.0))
    unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)
    assert unreal.EditorAssetLibrary.save_loaded_asset(blueprint, only_if_is_dirty=False)
    restored = unreal.get_default_object(blueprint.generated_class()).get_editor_property('mesh')
    assert abs(restored.get_editor_property('relative_location').z+55.0) < 0.001
    unreal.log('PLAYER_SPAWN_RESTORED '+str(original)+'; Mesh Z=-55cm')


if __name__ == '__main__':
    main()
