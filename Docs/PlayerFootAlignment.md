# 시작 시 발바닥 높이 보정

> **2026-11-18 현재 상태:** 사용자의 바닥 접지 수정 요청으로 Mesh Z를 실제 메시 최하단과 캡슐 바닥 여유 간격에서 계산한 **-61.819858cm**로 다시 맞췄다. 아래 복구 기록은 이전 작업 이력이다. PlayerStart의 저장 위치는 변경하지 않았다. 시작 모션의 `DisableMovement` 이전에 실제 바닥 충돌을 확인해 캡슐을 정렬하는 C++ 수정도 적용했다. PIE 시작은 PlayerStart 기준으로 고정했다. 검증 기록은 `Saved/MotionReference/startup_floor_validation.json`에 있다.

> **철회됨:** 아래 -61.81986cm 보정은 사용자의 복구 요청으로 되돌렸다.
> 현재 Mesh Z는 기존 **-55cm**이다. `align_player_feet.py`는 과거 보정 코드이며 재실행하지 않는다.
> 복구 전체 코드는 [Tools/Character/restore_player_spawn.py](../Tools/Character/restore_player_spawn.py)이다.
>
> 별도 읽기 전용 Unreal 프로세스로 저장된 원본 맵을 확인해 PlayerStart_1의 위치를
> **(1000, 1810, 1106.2458675782113)**으로 복구했다. 다른 레벨 액터는 수정하지 않았다.
> PIE 시작 기준은 현재 카메라가 아닌 PlayerStart로 설정했다.
> 복구 후 PIE에서 X=1000, Y=1810, 바닥 정착 후 캡슐 중심 Z 약 1063.400019cm를 확인했다.
> Mesh Z=-55cm, 캡슐 반높이=60cm, walkable floor=True, 바닥 간격 약 2.15cm이다.
> 발 접지에 대한 추가 추정 보정이나 새로운 충돌/중력 규칙은 적용하지 않았다.

## 아래 내용은 철회 전 기록

이번 변경은 `Content/Blueprints/Player/BP_LostArkCharacter.uasset`의 Mesh Relative Location Z를
기존 -55cm에서 약 -61.81986cm로 내린 것이다. X/Y, 회전, 스케일과 캡슐 반높이 60cm는 유지한다.
총은 메시의 손 본을 따라가므로 별도 위치 변경은 없다. 이동/공격/애니메이션은 변경하지 않는다.

현재 메시의 기준 포즈 최하단은 로컬 Z 약 -0.33014cm이며, CharacterMovement가 평지에서
유지하는 캡슐-바닥 간격 2.15cm를 함께 보정했다. 발 IK나 경사면 대응을 추가한 것은 아니다.

전체 설정 코드는 [Tools/Character/align_player_feet.py](../Tools/Character/align_player_feet.py)에 있다.
이 스크립트는 PIE 월드가 남아 있으면 중단하며, 저장 전에 Blueprint와 메시를 수정 대상으로
표시한다. 원본 메시와 캡슐의 충돌 설정은 수정하지 않는다.

에디터에서 추가로 적용할 설정은 없다. 다른 캐릭터 메시나 캡슐 크기를 쓰게 되면 높이를 다시
맞춰야 한다. PIE에서 시작 시 발 높이와 이동·슬라이드 후 접지를 확인할 수 있다.

이 과정에서 PIE 종료 요청 직후 상태 확인 없이 Blueprint 변경을 시도해 저장이 거부된 일이
있었으며, 이어 RiderLink 통신 코드에서 에디터가 종료됐다. 에디터 재실행 후 Idle 상태와
PIE 월드 없음 확인을 거쳐 다시 설정하고 저장했다. Python 실행 유지 설정은 사용하지 않는다.

효율성 리뷰에서 지적한 반복 로그, 클릭 이펙트 생성, 미니맵 갱신 등은 이번에 수정하지 않았다.
