# 시작 일어서기와 모션 보간

게임 시작 시 한쪽 무릎을 낮추고 왼손을 바닥에 짚은 자세에서 골반, 상체, 머리 순으로 일어나는 2.6초 원본 연출이다. 마지막 0.35초에는 대기 자세로 부드럽게 전환한다. 영화 장면을 참고한 절차적 모션 초안이며 촬영된 모션 캡처는 아니다.

## 시작 바닥 확인 수정

PlayerStart가 실제 바닥보다 높은 상태에서 시작 모션이 `DisableMovement`를 호출하여 낙하까지 멈추던 문제를 수정했다. `BeginPlay`에서 기존 캡슐 반경과 CharacterMovement의 `ComputeFloorDist`로 걸을 수 있는 바닥을 확인하고, 충돌 Sweep을 거쳐 바닥 여유 간격에 맞춘 뒤에만 이동을 완전히 잠근다. 바닥을 못 찾으면 입력 잠금만 적용하여 중력과 낙하가 유지된다. 검색 범위는 캡슐 전체 높이 + MaxStepHeight이며, PlayerStart의 저장 위치와 충돌 설정은 바꾸지 않는다.

PIE 설정이 `PlayLocation_CurrentCameraLocation`이면 에디터 카메라를 움직일 때마다 스폰 위치도 달라진다. PlayerStart 시작 설정으로 되돌렸으며 맵의 PlayerStart는 `PlayerStart_1` 한 개, 위치는 `(1000, 1810, 1106.245868)`이다. C++ 플레이어 무작위 스폰 코드는 없다. 정상 접지 후 캡슐 중심은 `(1000, 1810, 1063.400019)`, 바닥 간격은 약 `2.15cm`이다.

Rider `ue_play` 검증 호출은 `action="play"`일 때 반드시 `spawnAtPlayerStart=true`를 명시한다. 이 옵션을 생략한 검증 호출이 카메라 시작 설정으로 되돌리는 것을 확인했다. 일반 에디터 실행에서는 Play 메뉴의 Spawn Player At을 Default Player Start로 유지한다.

메시 자체도 기존 캡슐 바닥보다 약 6.82cm 높아, Blueprint Mesh Z를 메시의 실제 imported bounds 최하단과 캡슐 반높이, 정상 바닥 여유 간격으로 계산해 `-61.819858cm`로 맞췄다. 맵이나 PlayerStart 이동으로 보정하지 않았다. 시작부터 종료까지 바닥 간격과 위치가 유지되는 PIE 검증 및 가까운 측면 화면 확인을 완료했다. 검증 기록: `Saved/MotionReference/startup_floor_validation.json`.

## 적용 위치

- `Source/LostArk/public/Player/LostArkCharacter.h`: `StartupAnimation` 편집 가능한 참조 추가.
- `Source/LostArk/private/Player/LostArkCharacter.cpp`: `BeginPlay`에서 시작 시퀀스 길이만큼 기존 이동 잠금 사용. 종료 시 기존 타이머로 이동 복구.
- `Source/LostArk/private/Player/SorceressAnimInstance.cpp`: 시작 시퀀스를 인스턴스당 한 번 재생하고 대기/보행으로 전환. 속도에 따른 연속 블렌딩과 보행 재생 배속 보간 적용.
- 새 생성 도구 전체 코드: `Tools/Animation/create_smooth_player_motions.py`. 기존 생성기의 IK를 재사용하여 60fps 시퀀스와 가속도가 매끄러운 곡선을 생성한다. 걷기 발 높이 곡선은 지면 접촉 시 속도가 0이 되도록 보정한다.
- 새 애셋 12개: `Content/PlayerCharacter/Animations/SorceressSmooth/`. 기존 `Sorceress` 애셋은 보존한다. BombGun Montage는 원본을 복제하고 블렌딩 곡선만 조정한다.
- `Content/Blueprints/Player/BP_LostArkCharacter.uasset`: 새 시퀀스와 Montage 참조 연결.

## 에디터 설정

기본 플레이어 Blueprint에 연결을 완료했으므로 추가 설정은 필요 없다. 다른 캐릭터 Blueprint에서는 Mesh Anim Class를 기존 `SorceressAnimInstance`로 두고 `Animation > Startup > Startup Animation`을 `AS_Sorceress_Startup`으로 지정한다. 이 참조를 비우면 시작 연출과 추가 잠금을 끌 수 있다. 재생 중에는 기존 이동 잠금이 이동, 기본 공격, Bomb, 슬라이드를 막는다. 기존 컨트롤러 `StartDelay`는 독립적으로 유지된다.

플레이어 이동 속도, 공격 판정/잠금 시간, 충돌, 웨이브, Crystal, HUD, 미니맵 규칙은 이번 작업에서 수정하지 않았다. UE 5.6 지침과 실제 프로젝트 엔진 버전 5.8의 차이는 기존 상태이며 엔진 버전을 변경하지 않았다.

## 검증

- 에디터 종료 후 UE 5.8.4 `LostArkEditor Win64 Development` 정식 빌드 성공.
- 생성 commandlet 완료: 오류 0건. 기존 게임 로그 및 Paragon Rig 호환 경고가 출력된다.
- 추가 검증 기록: `Saved/MotionReference/smooth_validation.json`, `smooth_runtime_validation.json`.
- 전체 전투 입력, 실제 몬스터 피격/사망, 웨이브 전환, Crystal 파괴, HUD/미니맵 갱신 통합 플레이는 별도 확인이 필요하다. PIE에서 우클릭 이동, 좌클릭 공격, Space 슬라이드, BombGun 사용 후 Crystal 파괴와 웨이브 진행을 확인한다.

생성기는 기존 출력 경로가 있으면 중단한다. 재생성하려면 기존 결과를 보존하고 도구의 출력 폴더를 새 경로로 바꾼 뒤 실행한다.
