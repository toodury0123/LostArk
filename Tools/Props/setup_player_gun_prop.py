"""기존 Murdock의 총 지오메트리만 분리해 플레이어 오른손에 장식으로 연결한다.

Unreal Editor Python에서 실행한다. 원본 메시/스켈레톤/공격 규칙을 수정하지 않는다.
"""
import unreal

SOURCE = '/Game/ParagonMurdock/Characters/Heroes/Murdock/Meshes/Murdock'
OUTPUT = '/Game/Props/Weapons/SM_MurdockHandGun'
PLAYER = '/Game/Blueprints/Player/BP_LostArkCharacter'


def create_gun_mesh():
    # 이미 생성한 소품은 재사용하며 기존 애셋을 덮어쓰지 않는다.
    existing = unreal.load_asset(OUTPUT) if unreal.EditorAssetLibrary.does_asset_exist(OUTPUT) else None
    if existing is not None:
        assert isinstance(existing, unreal.StaticMesh), '소품 경로의 애셋 종류가 다릅니다.'
        return existing
    source = unreal.load_asset(SOURCE)
    mesh = unreal.DynamicMesh()
    options = unreal.GeometryScriptCopyMeshFromAssetOptions()
    options.set_editor_property('request_tangents', True)
    _, outcome = unreal.GeometryScript_AssetUtils.copy_mesh_from_skeletal_mesh(
        source, mesh, options, unreal.GeometryScriptMeshReadLOD())
    assert outcome == unreal.GeometryScriptOutcomePins.SUCCESS, '원본 메시 읽기 실패'
    bones = unreal.GeometryScript_BoneWeights.get_all_bones_info(mesh)[1]
    # 총 본에 스킨된 정점만 선택해 캐릭터 몸과 허리의 보조 무기를 제외한다.
    gun_bones = {b.index for b in bones if str(b.name) == 'gun' or str(b.name).startswith('gun_')}
    vertices = []
    for vertex in range(unreal.GeometryScript_MeshQueries.get_num_vertex_i_ds(mesh)):
        _, weight, valid = unreal.GeometryScript_BoneWeights.get_largest_vertex_bone_weight(mesh, vertex)
        if valid and weight.bone_index in gun_bones:
            vertices.append(vertex)
    assert vertices, '총 정점을 찾지 못했습니다.'
    _, selected = unreal.GeometryScript_MeshSelection.convert_index_array_to_mesh_selection(
        mesh, vertices, unreal.GeometryScriptMeshSelectionType.VERTICES)
    _, triangles = unreal.GeometryScript_MeshSelection.convert_mesh_selection(
        mesh, selected, unreal.GeometryScriptMeshSelectionType.TRIANGLES, False)
    _, unwanted = unreal.GeometryScript_MeshSelection.invert_mesh_selection(mesh, triangles)
    unreal.GeometryScript_MeshEdits.delete_selected_triangles_from_mesh(mesh, unwanted)
    # 원본 오른손을 피벗으로 사용하여 총의 손잡이 위치를 유지한다.
    hand = next(b.world_transform for b in bones if str(b.name) == 'hand_r')
    unreal.GeometryScript_MeshTransforms.inverse_transform_mesh(mesh, hand)
    materials = [m.get_editor_property('material_interface') for m in source.get_editor_property('materials')]
    _, compacted = unreal.GeometryScript_Materials.compact_material_i_ds(mesh, materials)
    create_options = unreal.GeometryScriptCreateNewStaticMeshAssetOptions()
    create_options.set_editor_property('enable_collision', False)
    create_options.set_editor_property('enable_nanite', False)
    create_options.set_editor_property('enable_recompute_normals', False)
    gun, outcome = unreal.GeometryScript_NewAssetUtils.create_new_static_mesh_asset_from_mesh(
        mesh, OUTPUT, create_options)
    assert outcome == unreal.GeometryScriptOutcomePins.SUCCESS, '총 Static Mesh 생성 실패'
    for index, material in enumerate(compacted):
        gun.set_material(index, material)
    unreal.EditorAssetLibrary.save_loaded_asset(gun)
    return gun


def main():
    blueprint = unreal.load_asset(PLAYER)
    cdo = unreal.get_default_object(blueprint.generated_class())
    assert cdo.get_editor_property('mesh').does_socket_exist('hand_r'), '오른손 본이 없습니다.'
    # 실제 본 부착은 C++의 GunProp 기본 컴포넌트가 담당한다.
    prop = cdo.get_editor_property('gun_prop')
    gun = create_gun_mesh()
    assert isinstance(prop, unreal.StaticMeshComponent)
    prop.set_static_mesh(gun)
    prop.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
    prop.set_editor_property('generate_overlap_events', False)
    prop.set_editor_property('can_ever_affect_navigation', False)
    prop.set_simulate_physics(False)
    prop.set_editor_property('mobility', unreal.ComponentMobility.MOVABLE)
    prop.set_editor_property('relative_location', unreal.Vector(0, 0, 0))
    prop.set_editor_property('relative_rotation', unreal.Rotator(0, 0, 0))
    prop.set_editor_property('relative_scale3d', unreal.Vector(0.55, 0.55, 0.55))
    unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)
    unreal.EditorAssetLibrary.save_loaded_asset(blueprint)
    unreal.log('GUN_PROP_SETUP_COMPLETE '+OUTPUT+' -> hand_r')


if __name__ == '__main__':
    main()
