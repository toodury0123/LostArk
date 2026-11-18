# BombGun 후퇴 반동

Bomb 사용이 승인되면 발사 방향의 반대로 기본 50cm 물러난다. 기존 공격 방향, 클릭한 피해 위치, 피해량·반경·발동 시간과 쿨다운은 유지한다. 구름 효과는 비활성화 상태를 유지한다.

## 조절 위치

`BP_LostArkCharacter > Class Defaults > Bomb > Recoil > Backward Distance`

C++ 변수 이름은 요청한 `float backwardDistance`이며 기본값은 50cm이다. 배치 인스턴스와 Blueprint에서도 조절할 수 있다. 0 또는 음수·유효하지 않은 값은 반동 이동과 작은 점프를 생략한다. 진행 중 값을 바꿔도 현재 반동은 시작 시의 거리를 유지한다.

## 변경 위치와 동작

- `Source/LostArk/public/Player/LostArkCharacter.h`: `backwardDistance` 편집 값, 반동 상태 및 시작·갱신·종료 함수 선언 추가.
- `Source/LostArk/private/Player/LostArkCharacter.cpp`: `BombAttack`에서 발사 방향을 정한 뒤 `StartBombRecoil` 호출. 기존 Tick은 반동이 진행 중일 때만 `UpdateBombRecoil`을 호출한다. `LockMovement`, `EnableMovement`, `EndPlay`에서 반동을 정리한다.

0.28초 동안 부드러운 곡선으로 후퇴하며, 메시만 최대 8cm 올렸다 내린다. 캡슐은 실제 바닥 간격을 유지하므로 카메라·미니맵 높이와 충돌 설정은 바뀌지 않는다. 시작 바닥과 NavMesh를 확인하고, 이동 중 매 단계에서 걸을 수 있는 바닥과 NavMesh 경계를 검사한다. 캡슐 Sweep으로 벽·액터 충돌을 확인하며 막히면 즉시 멈추고 메시 높이를 복구한다. 바닥이나 이동 가능한 NavMesh가 없는 시작점에서는 반동을 시작하지 않는다. 반동이 막혀도 Bomb 피해는 원래 클릭한 위치에 적용된다.

실제 플레이어 스켈레톤의 `AS_Sorceress_BombRecoil` 시퀀스는 총을 들고 상체가 반동을 받으며 무릎을 굽혔다 착지하는 자세를 포함한다. `SorceressAnimInstance`가 후퇴 진행률과 같은 시간의 자세를 평가하고 대기 자세와 부드럽게 혼합한다. 루트 이동과 피해 Notify는 사용하지 않는다. 애니메이션 참조는 `Animation > Bomb > Bomb Recoil Animation`에 연결한다.

새 애셋은 `Content/PlayerCharacter/Animations/SorceressSmooth/AS_Sorceress_BombRecoil.uasset`이다. 생성·연결 도구 전체 코드는 [Tools/Animation/create_bomb_recoil_motion.py](../Tools/Animation/create_bomb_recoil_motion.py)에 있다. 원본 모션 애셋은 수정하지 않는다.

새 모션 애셋을 연결했으므로 추가 에디터 설정은 필요 없다. 모듈·플러그인과 실제 프로젝트의 UE 5.8 엔진 설정은 유지한다.

이전에 요청한 폭발 구름 비활성화가 Blueprint 재컴파일 뒤에도 유지되도록 `Bomb > Visual > Enable Bomb Explosion Cloud`는 기본 false이다. 구름 클래스 참조가 남아 있어도 이 옵션이 꺼져 있으면 표시하지 않는다.

## 검증 경로

에디터 종료 후 `LostArkEditor Win64 Development` 정식 빌드 통과. 실제 Bomb 사용의 거리·작은 점프·충돌·NavMesh·바닥 검증 코드는 `Saved/MotionReference/check_bomb_recoil.py`, 결과는 `bomb_recoil_validation.json`이다. 실제 몬스터 피해 회귀 검증은 `check_bomb_damage.py`와 `bomb_damage_validation.json`을 사용한다.

PIE 검증 통과: 설정 50cm와 100cm에서 각각 정확히 50cm와 100cm 후퇴. 0cm는 이동·점프 없음. 점프 중간 프레임은 약 7.64cm 상승했고 종료 시 원래 메시 높이로 복구됐다. 실제 벽 앞에서는 0cm, NavMesh 경계 앞에서는 8cm 이동 후 멈췄으며, 바닥 없는 위치에서는 시작하지 않았다. 접지 구간의 캡슐 바닥 간격은 약 2.15cm로 유지됐다. 테스트는 맵에 저장하지 않고 PIE 안에서만 수행했다.

실제 반동 시퀀스를 연결한 최종 PIE 검증에서도 위 6가지 경우를 모두 통과했다. 오른손 본은 대기 자세에서 약 14.8cm 변해 새 발사·점프 자세가 실제 그래프에 반영됨을 확인했다. 반동 추가 후 실제 몬스터 피해 37·83, 범위 밖 피해 없음, 중복 피해 방지, 사망 집계 2마리 회귀 검증도 통과했다. 수동 키 입력 연계·경사면·전체 웨이브 완료 회귀는 미실행이며 PIE에서 Bomb과 이동·슬라이드를 연계해 확인한다.
