# 플레이어 오른손 총 소품

기존 Murdock Skeletal Mesh에서 `gun`과 하위 본에 스킨된 지오메트리만 분리해
`Content/Props/Weapons/SM_MurdockHandGun.uasset`을 생성했다. 원본 캐릭터와 허리의
보조 무기는 포함하지 않으며, 원본 메시와 재질은 변경하지 않는다.

`BP_LostArkCharacter`의 `GunProp` 컴포넌트에 이 Static Mesh를 연결했다.
부착 부모는 캐릭터 Mesh이며 본 이름은 `hand_r`이다. 원본 오른손을 메시 피벗으로
사용하고 상대 스케일은 0.55로 설정했다. 상대 위치/회전은 0이다.
충돌, 오버랩, 물리 시뮬레이션과 내비게이션 영향은 꺼져 있다.

## 변경 위치

- `Source/LostArk/public/Player/LostArkCharacter.h`: Equipment 카테고리의 `GunProp` 선언 추가.
- `Source/LostArk/private/Player/LostArkCharacter.cpp`: 생성자에서 장식 컴포넌트를 만들고
  `hand_r`에 부착. 애셋 경로는 C++에 넣지 않고 Blueprint에서 지정한다.
- `Content/Blueprints/Player/BP_LostArkCharacter.uasset`: GunProp 메시/보정값 저장.
- `Content/Props/Weapons/SM_MurdockHandGun.uasset`: 분리된 총 소품 메시와 기존 재질 참조.
- [Tools/Props/setup_player_gun_prop.py](../Tools/Props/setup_player_gun_prop.py): 분리 및 연결
  전체 코드. 기존 출력 애셋은 재사용하고 덮어쓰지 않는다. Unreal Editor Python에서 실행한다.

## 에디터 설정

추가 설정 없이 적용되어 있다. 위치를 다듬으려면 `BP_LostArkCharacter`의 Components에서
`GunProp`을 선택하고 Relative Location/Rotation/Scale을 조절한다. 총 메시도 이곳에서
다른 Static Mesh로 교체할 수 있다.

발사/투사체/손가락 쥐기/왼손 IK는 이번 요청에 포함하지 않았다. 기존 공격과 BombGun의
판정, 재생, Notify, 입력, 충돌, HUD, 미니맵, 웨이브 규칙은 변경하지 않았다.

## 검증

- 에디터 종료 후 UE 5.8.4 LostArkEditor Win64 Development 정식 빌드 성공.
- 저장된 Blueprint의 Static Mesh 참조, Mesh 부모, hand_r 부착, 충돌 꺼짐, 0.55 스케일 확인.
- 실행 중 PIE 플레이어에서도 메시 참조, 부모/본 부착, 표시 상태와 충돌 꺼짐 확인.
  총 피벗과 hand_r의 월드 위치 차이는 0cm이다. 손잡이 회전의 시각적 미세 조정은 별도이다.
- Python Keep Alive는 False이며 검사 종료 후 실행 유지 작업을 남기지 않는다.
- 전체 전투 회귀 확인은 미실행. PIE에서 이동/공격/슬라이드/BombGun 중 소품의 손 추종과
  충돌 없음, HUD·미니맵·웨이브 진행을 확인할 수 있다.
