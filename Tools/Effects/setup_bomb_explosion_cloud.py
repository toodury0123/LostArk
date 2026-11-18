"""주황 외곽/노란 중심의 폭발 구름 재질과 Blueprint를 만들고 BombGun에 연결한다."""
import unreal

FOLDER = '/Game/Effects/BombCloud'
MATERIAL_PATH = FOLDER+'/M_BombExplosionCloud'
BLUEPRINT_PATH = FOLDER+'/BP_BombExplosionCloud'
LIB = unreal.MaterialEditingLibrary


def create_material():
    if unreal.EditorAssetLibrary.does_asset_exist(MATERIAL_PATH):
        return unreal.load_asset(MATERIAL_PATH)
    tools = unreal.AssetToolsHelpers.get_asset_tools()
    material = tools.create_asset('M_BombExplosionCloud', FOLDER, unreal.Material, unreal.MaterialFactoryNew())
    material.set_editor_property('blend_mode', unreal.BlendMode.BLEND_TRANSLUCENT)
    material.set_editor_property('shading_model', unreal.MaterialShadingModel.MSM_UNLIT)
    material.set_editor_property('two_sided', False)
    material.set_editor_property('used_with_instanced_static_meshes', True)

    def node(cls, x, y, **properties):
        expression = LIB.create_material_expression(material, cls, x, y)
        for name, value in properties.items():
            expression.set_editor_property(name, value)
        return expression

    def connect(source, target, pin, output=''):
        connected = LIB.connect_material_expressions(source, output, target, pin)
        if not connected and pin == 'Input':
            connected = LIB.connect_material_expressions(source, output, target, '')
        assert connected, '재질 연결 실패: '+pin

    def scalar(name, value, x, y):
        return node(unreal.MaterialExpressionScalarParameter, x, y, parameter_name=name, default_value=value)

    def color(name, value, x, y):
        return node(unreal.MaterialExpressionVectorParameter, x, y, parameter_name=name, default_value=value)

    # 클릭 중심에서의 수평 거리에 따라 노란 중심을 주황 외곽으로 혼합한다.
    world = node(unreal.MaterialExpressionWorldPosition, -1500, -500)
    center = color('ExplosionCenter', unreal.LinearColor(0, 0, 0, 1), -1500, -300)
    subtract = node(unreal.MaterialExpressionSubtract, -1250, -500)
    connect(world, subtract, 'A')
    connect(center, subtract, 'B')
    xy = node(unreal.MaterialExpressionComponentMask, -1050, -500, r=True, g=True)
    connect(subtract, xy, 'Input')
    dot = node(unreal.MaterialExpressionDotProduct, -850, -500)
    connect(xy, dot, 'A')
    connect(xy, dot, 'B')
    distance = node(unreal.MaterialExpressionSquareRoot, -650, -500)
    connect(dot, distance, 'Input')
    radius = scalar('ExplosionRadius', 500.0, -850, -300)
    divide = node(unreal.MaterialExpressionDivide, -430, -500)
    connect(distance, divide, 'A')
    connect(radius, divide, 'B')
    radial = node(unreal.MaterialExpressionSaturate, -220, -500)
    connect(divide, radial, 'Input')
    inner = color('InnerColor', unreal.LinearColor(1, 0.85, 0.035, 1), -430, -150)
    outer = color('OuterColor', unreal.LinearColor(1, 0.18, 0.015, 1), -430, 50)
    blend = node(unreal.MaterialExpressionLinearInterpolate, 30, -300)
    connect(inner, blend, 'A')
    connect(outer, blend, 'B')
    connect(radial, blend, 'Alpha')
    assert LIB.connect_material_property(blend, '', unreal.MaterialProperty.MP_EMISSIVE_COLOR)

    # 부드러운 구 표면과 기존 타일 연기 텍스처를 섞어 단단한 공처럼 보이지 않게 한다.
    fresnel = node(unreal.MaterialExpressionFresnel, -1300, 350, exponent=1.4, base_reflect_fraction=0.0)
    soft_edge = node(unreal.MaterialExpressionOneMinus, -1050, 350)
    connect(fresnel, soft_edge, 'Input')
    noise = node(unreal.MaterialExpressionTextureSampleParameter2D, -1250, 700,
                 parameter_name='CloudNoise', texture=unreal.load_asset(
                     '/Game/ParagonMurdock/FX/Textures/Tile/Noise/T_Inky_Smoke_Tile'))
    uv = node(unreal.MaterialExpressionTextureCoordinate, -1700, 700, u_tiling=1.2, v_tiling=1.2)
    age = scalar('CloudAge', 0.0, -1900, 1000)
    direction = node(unreal.MaterialExpressionConstant2Vector, -1900, 1150, r=0.12, g=0.08)
    drift = node(unreal.MaterialExpressionMultiply, -1670, 1000)
    connect(age, drift, 'A')
    connect(direction, drift, 'B')
    moving_uv = node(unreal.MaterialExpressionAdd, -1450, 900)
    connect(uv, moving_uv, 'A')
    connect(drift, moving_uv, 'B')
    connect(moving_uv, noise, 'UVs')
    noise_scale = node(unreal.MaterialExpressionMultiply, -950, 700, const_b=0.6)
    connect(noise, noise_scale, 'A', 'R')
    density = node(unreal.MaterialExpressionAdd, -740, 700, const_b=0.35)
    connect(noise_scale, density, 'A')
    masked = node(unreal.MaterialExpressionMultiply, -500, 450)
    connect(soft_edge, masked, 'A')
    connect(density, masked, 'B')
    opacity = scalar('CloudOpacity', 1.0, -500, 800)
    fade = node(unreal.MaterialExpressionMultiply, -250, 450)
    connect(masked, fade, 'A')
    connect(opacity, fade, 'B')
    density_scale = node(unreal.MaterialExpressionMultiply, -40, 450, const_b=0.72)
    connect(fade, density_scale, 'A')
    depth_fade = node(unreal.MaterialExpressionDepthFade, 180, 450, fade_distance_default=40.0)
    connect(density_scale, depth_fade, 'Opacity')
    assert LIB.connect_material_property(depth_fade, '', unreal.MaterialProperty.MP_OPACITY)
    LIB.layout_material_expressions(material)
    LIB.recompile_material(material)
    unreal.EditorAssetLibrary.save_loaded_asset(material, only_if_is_dirty=False)
    return material


def configure_sprite_material(material):
    # 구 표면용 Fresnel만으로는 평면 스프라이트의 사각 경계가 가려지지 않는다.
    material.modify()
    material.set_editor_property('used_with_niagara_sprites', True)
    marker = 'BombCloudSpriteOpacity'
    if not any(n.get_editor_property('desc') == marker for n in LIB.get_material_expressions(material)):
        original = LIB.get_material_property_input_node(material, unreal.MaterialProperty.MP_OPACITY)
        uv = LIB.create_material_expression(material, unreal.MaterialExpressionTextureCoordinate)
        center = LIB.create_material_expression(material, unreal.MaterialExpressionConstant2Vector)
        center.set_editor_property('r', 0.5)
        center.set_editor_property('g', 0.5)
        mask = LIB.create_material_expression(material, unreal.MaterialExpressionSphereMask)
        mask.set_editor_property('attenuation_radius', 0.5)
        mask.set_editor_property('hardness_percent', 0.0)
        assert LIB.connect_material_expressions(uv, '', mask, 'A')
        assert LIB.connect_material_expressions(center, '', mask, 'B')
        masked = LIB.create_material_expression(material, unreal.MaterialExpressionMultiply)
        assert LIB.connect_material_expressions(original, '', masked, 'A')
        assert LIB.connect_material_expressions(mask, '', masked, 'B')
        particles = LIB.create_material_expression(material, unreal.MaterialExpressionParticleColor)
        fade = LIB.create_material_expression(material, unreal.MaterialExpressionMultiply)
        fade.set_editor_property('desc', marker)
        assert LIB.connect_material_expressions(masked, '', fade, 'A')
        assert LIB.connect_material_expressions(particles, 'A', fade, 'B')
        assert LIB.connect_material_property(fade, '', unreal.MaterialProperty.MP_OPACITY)
        LIB.layout_material_expressions(material)
    LIB.recompile_material(material)
    assert unreal.EditorAssetLibrary.save_loaded_asset(material)


def main():
    assert not unreal.EditorLevelLibrary.get_pie_worlds(False), 'PIE를 먼저 완전히 종료하세요.'
    material = create_material()
    configure_sprite_material(material)
    system_path = FOLDER+'/NS_BombExplosionCloud'
    system = unreal.load_asset(system_path) if unreal.EditorAssetLibrary.does_asset_exist(system_path) else None
    if system is None:
        # 기존 작성 라이브러리로 일회성 폭발 구름을 생성하고 누락된 시스템 참조를 복구한다.
        system, report = unreal.BombNiagaraAuthoringLibrary.build_bomb_cloud_system(
            unreal.load_asset('/Niagara/DefaultAssets/Templates/Emitters/SimpleSpriteBurst'),
            unreal.load_asset('/Niagara/Modules/Spawn/Location/V2/ShapeLocation'),
            material, system_path)
        if system is None:
            raise RuntimeError('폭발 구름 시스템 생성 실패: '+report)
        assert unreal.EditorAssetLibrary.save_loaded_asset(system)
    blueprint = unreal.load_asset(BLUEPRINT_PATH) if unreal.EditorAssetLibrary.does_asset_exist(BLUEPRINT_PATH) else None
    if blueprint is None:
        factory = unreal.BlueprintFactory()
        factory.set_editor_property('parent_class', unreal.load_class(None, '/Script/LostArk.BombExplosionCloud'))
        blueprint = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
            'BP_BombExplosionCloud', FOLDER, unreal.Blueprint, factory)
    blueprint.modify()
    effect = unreal.get_default_object(blueprint.generated_class())
    effect.modify()
    effect.set_editor_property('cloud_mesh', unreal.load_asset('/Engine/BasicShapes/Sphere'))
    effect.set_editor_property('cloud_material', material)
    effect.set_editor_property('cloud_system', system)
    effect.set_editor_property('radius', 500.0)
    effect.set_editor_property('duration', 1.0)
    unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)
    unreal.EditorAssetLibrary.save_loaded_asset(blueprint, only_if_is_dirty=False)
    player = unreal.load_asset('/Game/Blueprints/Player/BP_LostArkCharacter')
    player.modify()
    character = unreal.get_default_object(player.generated_class())
    character.modify()
    character.set_editor_property('bomb_cloud_effect_class', blueprint.generated_class())
    # 피해 반경(BombRadius)은 유지하고 표시 반경은 BombExplosionCloudRange에서 조절한다.
    character.set_editor_property('bomb_effect_duration', 1.0)
    character.set_editor_property('bomb_outer_color', unreal.LinearColor(1, 0.18, 0.015, 1))
    character.set_editor_property('bomb_inner_color', unreal.LinearColor(1, 0.85, 0.035, 1))
    unreal.BlueprintEditorLibrary.compile_blueprint(player)
    unreal.EditorAssetLibrary.save_loaded_asset(player, only_if_is_dirty=False)
    unreal.log('BOMB_CLOUD_SETUP_COMPLETE visual_range='+str(character.get_editor_property('bomb_explosion_cloud_range')))


if __name__ == '__main__':
    main()
