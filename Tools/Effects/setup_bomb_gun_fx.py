"""기존 Murdock 산탄총 효과를 플레이어 GunProp에 연결한다. 피해·반경은 유지한다."""
import unreal


def main():
    assert not unreal.EditorLevelLibrary.get_pie_worlds(False), 'PIE 종료 후 실행하세요.'
    bp = unreal.load_asset('/Game/Blueprints/Player/BP_LostArkCharacter')
    effect = unreal.load_asset('/Game/ParagonMurdock/FX/Particles/Abilities/SpreadShot/FX/P_Murdock_Spreadshot')
    assert isinstance(effect, unreal.ParticleSystem), '기존 산탄총 발사 효과가 없습니다.'
    character = unreal.get_default_object(bp.generated_class())
    gun = character.get_editor_property('gun_prop').get_editor_property('static_mesh')
    assert gun is not None, '플레이어 총 소품이 없습니다.'
    bounds = gun.get_bounding_box()
    # 오른손 기준 총 메시의 긴 축은 -X이며, 총열 끝에서 바깥쪽으로 효과를 발사한다.
    muzzle = unreal.Vector(bounds.min.x+2, (bounds.min.y+bounds.max.y)/2, (bounds.min.z+bounds.max.z)/2)
    bp.modify()
    character.modify()
    character.set_editor_property('bomb_muzzle_effect', effect)
    character.set_editor_property('bomb_muzzle_transform', unreal.Transform(muzzle, unreal.Rotator(0, 180, 0), unreal.Vector(1, 1, 1)))
    unreal.BlueprintEditorLibrary.compile_blueprint(bp)
    assert unreal.EditorAssetLibrary.save_loaded_asset(bp)
    unreal.log('BOMB_GUN_FX_CONNECTED '+effect.get_path_name())


if __name__ == '__main__':
    main()
