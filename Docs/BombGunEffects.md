# BombGun 기존 효과 연결

## 현재 상태: 폭발 구름 비활성화와 피해 호출 복구

사용자 요청으로 `Enable Bomb Explosion Cloud`를 기본 false로 두어 폭발 구름 표시를 껐다. Blueprint 재컴파일로 구름 클래스 참조가 복원돼도 이 옵션이 꺼져 있으면 표시하지 않는다. 구름 애셋과 범위 설정은 보존하며 총구 효과는 유지한다. 아래 구름 연결·범위 기록은 이전 작업 이력이다.

`BombAttack`은 활성 상태만 설정하고 `BombHit` 호출을 기존 애니메이션에 맡기고 있었다. 현재 플레이어는 `SorceressAnimInstance`를 사용하고, Bomb Montage는 다른 Murdock 스켈레톤을 참조한다. 현재 C++ 애님 클래스에는 기존 `AttackBomb` 알림의 피해 전달 처리도 없어 사용 효과만 보이고 `ApplyRadialDamage`가 호출되지 않을 수 있었다.

`Source/LostArk/private/Player/LostArkCharacter.cpp`의 `BombAttack`에서 Montage의 기존 `AttackBomb` 시점을 읽어 `BombHit`을 예약하도록 수정했다. 알림이 없으면 다음 틱에 적용한다. 기존 Blueprint 알림이 먼저 도착해도 `BombHit`의 활성 상태 검사와 타이머 취소로 중복 피해를 막는다. `EndPlay`에서 새 타이머도 정리한다. 헤더에는 `BombDamageTimerHandle`만 추가했으며, 기존 피해량·피해 반경·충돌·쿨다운·웨이브 규칙은 바꾸지 않았다.

데미지는 플레이어 Blueprint 또는 배치 인스턴스의 `Bomb > Bomb Damage`, 피해 반경은 `Bomb Radius`에서 조절한다. 저장된 기본값은 피해량 100, 피해 반경 300cm이다. 구름 표시 반경은 피해 범위와 무관하다. 검증 스크립트와 결과는 `Saved/MotionReference/check_bomb_damage.py`, `bomb_damage_validation.json`이다.

에디터를 종료한 정식 Editor 타깃 빌드 통과. 실제 Crystal을 파괴해 생성된 웨이브 몬스터에서 피해 37 적용 시 체력 1000→963, 이어 피해 83 적용 시 963→880을 확인했다. 범위 밖 몬스터는 1000을 유지했고, 추가 `BombHit` 호출에 중복 피해가 없었다. 피해 2000으로 범위 안 두 몬스터가 사망하면서 처치 수가 2 증가했고, Output Log의 HUD 갱신도 2/19, 비율 0.105263으로 일치했다. 구름 비활성화 상태에서도 피해가 적용되며, 저장된 기본 피해량 100과 피해 반경 300cm는 유지했다. 벽 뒤 대상 차폐·전체 웨이브 완료·미니맵 전투 회귀는 미실행이다.

기존 Murdock 산탄총 파티클 `P_Murdock_Spreadshot`이 프로젝트에 있어 플레이어의 `GunProp` 총구에 연결했다. 허용된 Bomb 사용 시 한 번 생성되며, 총 소품에 부착되어 손의 움직임을 따라간다. 파티클은 기존 자동 종료/정리를 사용한다.

폭발 효과는 직접 만든 `BP_BombExplosionCloud`와 `NS_BombExplosionCloud`를 사용한다. 사용자 요청으로 기존 `BombEffect` 항목과 `RadialBurst` 대체 경로를 제거했다. `CloudSystem`의 누락된 연결을 복구하고, 구름은 실제 Bomb 사용 시 클릭한 지면에 한 번 표시한다. 기존 총구 효과는 유지한다.

`BP_LostArkCharacter`의 Class Defaults → `Bomb > Visual > Bomb Explosion Cloud Range`에서 구름의 수평 **반경(cm)** 을 조절한다. 기본값은 500cm(지름 10m)이다. 값이 두 배면 구름 표시 범위도 두 배가 된다. `Bomb Radius`는 기존 피해 판정 반경이며 이 값과 독립적이다. 지속 시간과 외곽/중심 색상은 기존 설정을 사용한다.

## 변경 파일과 설정

- `Source/LostArk/public/Player/LostArkCharacter.h`: `BombMuzzleEffect`, `BombMuzzleTransform`, `BombExplosionCloudRange` 편집 설정. 기존 `BombEffect` 제거.
- `Source/LostArk/private/Player/LostArkCharacter.cpp`: `BombAttack`에서 총구 효과와 구름만 생성하고 구름에 독립 표시 반경 전달. `BombHit`은 기존 피해 판정을 유지한다.
- `Content/Blueprints/Player/BP_LostArkCharacter.uasset`: `Bomb > Visual > Bomb Muzzle Effect`에 기존 산탄총 파티클 연결. 총구 보정은 GunProp 메시의 -X 끝을 기준으로 계산한다.
- 새 연결 도구 전체 코드: [Tools/Effects/setup_bomb_gun_fx.py](../Tools/Effects/setup_bomb_gun_fx.py). Unreal Python에서 실행하며 기존 파티클 원본은 수정하지 않는다.
- 구름 생성·연결 도구 전체 코드: [Tools/Effects/setup_bomb_explosion_cloud.py](../Tools/Effects/setup_bomb_explosion_cloud.py). 새 애셋 경로는 `Content/Effects/BombCloud/NS_BombExplosionCloud.uasset`이며 구름 재생을 위해 추가했다. 기존 `BP_BombExplosionCloud`와 플레이어 Blueprint에 연결한다.

추가 에디터 설정은 필요 없다. 다른 총 메시를 사용하면 `Bomb Muzzle Transform`의 위치·회전·스케일을 조정한다. 기존 데미지, 반경, 재사용 시간, Bomb Montage와 Notify 소유권은 유지한다. 총구 섬광은 실제 Bomb 사용 승인 시점의 시각 효과이며 별도 투사체나 피해 판정을 추가하지 않는다.

## 확인 기록

에디터 종료 후 UE 5.8.4 `LostArkEditor Win64 Development` 정식 빌드 성공. PIE 자동 검증은 `Saved/MotionReference/check_bomb_gun_fx.py`, 결과는 `bomb_gun_fx_validation.json`을 사용한다. 마우스·키보드 입력부터 전체 몬스터 전투 및 웨이브/UI 갱신까지의 통합 확인은 미실행이며 PIE에서 Bomb 입력으로 확인한다.

이전 총구 효과 검증에서는 실제 C++ `BombAttack` 경로의 파티클 1개 생성·활성화·GunProp 부착, 쿨다운 중 재사용 차단, 이동 복구, 바닥 간격 2.15cm를 확인했다. 이전 화면 기록은 `Saved/Screenshots/WindowsEditor/RiderMCP/20261118-065856_viewport.png`이다. 현재 구름 전용 변경의 검증 기록은 `Saved/MotionReference/bomb_cloud_range_validation.json`을 사용한다.

구름 전용 변경의 정식 빌드 및 PIE 검증 통과: 반경 250cm → Niagara 배율 0.5, 750cm → 배율 1.5. 각 사용에서 구름 액터 1개 생성·활성화 및 수명 종료 후 정리를 확인했다. 피해 반경은 기존 300cm, 바닥 간격은 2.15cm로 유지됐다. 저장된 기본 구름 반경은 500cm이며 검증 후 실행 중 임시 변경값도 복구했다. 전체 몬스터 전투 회귀 확인은 미실행이다.
