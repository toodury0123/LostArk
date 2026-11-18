# 소서리스풍 모션 초안

참고 영상: `C:/Users/toodu/Videos/PlaySorcer.mp4` (30fps, 약 12.07초).
영상에서 보이는 이동/시전 흐름을 참고해 기준 포즈와 IK로 새로 만든 애니메이션이다.
원작 모션 데이터나 모션 캡처를 사용하지 않았으며, 이펙트에 가려진 자세는 추정했다.
대기/피격/넘어짐은 영상에서 명확히 확인되지 않아 같은 스타일로 구성한 초안이다.

## 생성 경로와 연결

애셋 폴더: `Content/PlayerCharacter/Animations/Sorceress/`.
현재 `PlayerCharacter_Final` 메시의 `PlayerCharacter_Skeleton`을 직접 사용한다.
기존 메시, Skeleton, 재질은 변경하지 않는다.

| 동작 | 애니메이션 | 길이 | 사용 지점 |
|---|---|---|---|
| 대기 | AS_Sorceress_Idle | 2.4초 | 반복, 속도 0 |
| 걷기 | AS_Sorceress_Walk | 1.0초 | 이동 중 반복, 재생 속도 조절 |
| 슬라이드 | AS_Sorceress_Slide / AM_Sorceress_Slide | 0.4초 | 기존 SlideMontage |
| 피격 | AS_Sorceress_Hit / AM_Sorceress_Hit | 0.4초 | SetHP의 비치명적 체력 감소 |
| 넘어짐/일어나기 | AS_Sorceress_FallDown / AM_Sorceress_FallDown | 2.3초 | PlayFallDownMotion 명시적 호출 |
| 파이어볼 시전 | AS_Sorceress_Fireball / AM_Sorceress_Fireball | 0.4초 | 기존 AttackMontage1/2 |

`BP_LostArkCharacter`의 Mesh Animation Class는 `SorceressAnimInstance`이다.
이 C++ 애님 클래스는 Idle/Walk를 부드럽게 혼합하고 `DefaultSlot`으로 Montage를 출력한다.
애셋 참조는 캐릭터의 Class Defaults에서 편집할 수 있다.
`WalkAnimationSpeed`는 애니메이션 재생 배속의 기준이며 실제 이동 속도를 변경하지 않는다.
모든 새 Montage는 DefaultSlot을 사용한다. 슬라이드 이동은 기존 C++ 코드가 담당하며
애니메이션 Root Motion은 꺼져 있다.

피격은 공격/스킬/슬라이드/넘어짐 잠금 중에는 다른 Montage를 중단시키지 않는다.
체력 계산, 공격 데미지, 입력, 충돌, 웨이브, Crystal, HUD, 미니맵은 기존 처리를 사용한다.
BombMontage와 BombHit의 기존 Blueprint 재생/Notify 연결은 변경하지 않는다.

넘어짐은 **넘어졌다가 일어나기**로 만들었다. 새로운 사망 규칙, 넘어짐 판정, 키는 추가하지
않았다. 필요한 이벤트에서 캐릭터의 `PlayFallDownMotion()`을 호출한다. 동작 중에는 기존
이동 잠금을 사용하고, 다른 행동이 잠겨 있으면 호출을 무시한다.

파이어볼은 **시전 모션**이다. 기존 기본 공격 판정은 즉시 실행된다. 새 투사체, 화염 이펙트,
발사 지연이나 추가 데미지는 구현하지 않았다. 영상과 같은 무기/지팡이도 추가하지 않았다.

## 소스 변경 위치와 전체 코드

- `Source/LostArk/public/Player/SorceressAnimInstance.h`: 새 애님 클래스 선언 전체.
- `Source/LostArk/private/Player/SorceressAnimInstance.cpp`: Idle/Walk/Slot 그래프와 속도 갱신 전체.
- `Source/LostArk/public/Player/LostArkCharacter.h`: Animation 카테고리의 애셋 참조와 호출 함수 추가.
- `Source/LostArk/private/Player/LostArkCharacter.cpp`: SetHP 피격 연출과 PlayFallDownMotion 추가.
- `Source/LostArk/LostArk.Build.cs`: 위 C++ 애니메이션 노드 사용을 위해 AnimGraphRuntime의
  private 의존성 1개 추가. 플러그인/uproject 설정은 변경하지 않는다.
- `Tools/Animation/create_sorceress_motions.py`: 생성 및 Blueprint 연결 코드 전체.
  Unreal Python 스크립트이며 기존 결과 경로가 있으면 덮어쓰지 않고 중단한다.
- `Tools/Animation/validate_sorceress_motions.py`: 저장된 애셋 재로드 검증 코드 전체.

## 확인 절차

1. 에디터가 종료된 상태에서 LostArkEditor Win64 Development를 정식 빌드한다.
2. 기존 게임 맵을 열고 BP_LostArkCharacter의 위 애니메이션 참조를 확인한다.
3. PIE에서 정지/우클릭 이동 전환, 좌클릭 공격, 기존 슬라이드 입력을 확인한다.
4. 몬스터에게 피격되어 짧은 피격 모션이 재생되는지 확인한다. 공격/스킬 중 피격으로
   Bomb Montage 또는 BombHit Notify가 끊기지 않는지 확인한다.
5. 임시 Blueprint 이벤트에서 PlayFallDownMotion을 호출해 넘어짐/일어나기와 이동 복구를
   확인한다. 검증용 새 입력은 이 작업에서 추가하지 않았다.
6. BombGun, 몬스터 생성/사망, 웨이브 전환, Crystal, HUD/미니맵 갱신을 확인한다.

실제 프로젝트와 README는 UE 5.8이고 AGENTS.md에는 UE 5.6이 명시되어 있다.
기존 프로젝트의 UE 5.8.4로 빌드/애셋 생성을 진행했으며 엔진 버전을 변경하지 않았다.

검증 기록은 `Saved/MotionReference/`에 보관한다. OpenCV는 참고 영상 프레임 추출용으로
`Saved/MotionTools/python`에만 설치했으며 게임 실행에는 필요하지 않다.

촬영용 `-ExecutePythonScript` 실행에서는 비동기 캡처를 기다리는 동안에만
`EditorPythonScripting.set_keep_python_script_alive(True)`를 사용한다. 완료/오류 시 반드시
False로 되돌려 검사 에디터를 종료한다. 일반 작업은 스크립트 인자 없이 에디터를 실행한다.
검사 뒤 유지 설정을 해제하지 않아 실행 중 알림이 남았던 문제를 수정했다.

## 이번 작업의 검증 결과

- UE 5.8.4 LostArkEditor Win64 Development 정식 빌드 성공. 에디터 종료 상태에서 실행했다.
- 저장된 6개 시퀀스와 4개 Montage의 Skeleton/길이/슬롯/Blueprint 연결 검증 통과.
- Idle/Walk 반복 경계와 원본 대비 압축 데이터의 주요 관절 위치 오차 0.5cm 미만 검증 통과.
- 실제 메시 미리보기에서 보행, 굽힌 자세, 피격, 넘어짐, 시전 자세를 확인했다.
  미리보기 촬영 과정에서 에디터의 게임 스레드/뷰포트 realtime ensure가 발생했으며,
  촬영용 도구 문제와 구분하기 위해 이후 PIE에서 애님 그래프를 별도로 검사했다.
- 기본 맵 PIE에서 SorceressAnimInstance 생성과 대기 자세, HUD 생성 확인.
- 기존 내비게이션으로 약 1436cm 이동, 최대 속도 600cm/s, 걷기 중 손 위치 변화 약 13.9cm 확인.
- PlayFallDownMotion 호출 시 MOVE_NONE, 0.6초 뒤 골반 높이 약 15.7cm, 종료 뒤 MOVE_WALKING
  복구를 확인했다. `Saved/MotionReference/runtime_validation.json`에 결과를 기록했다.
- 관련 기존 자동화 테스트는 Source 검색에서 찾지 못했다.
- **미실행:** 마우스/키보드로 공격·슬라이드·BombGun을 사용하는 전체 전투 회귀 확인,
  실제 몬스터 피격 상황, 웨이브 전환/Crystal 파괴/몬스터 사망에 따른 HUD·미니맵 갱신.
  위 확인 절차 3~6을 PIE에서 수행해야 한다. 이번 자동 점검은 애님 그래프와 이동/복구에
  한정했으며 기존 게임 루프 전체의 검증을 대신하지 않는다.

걷기 모션은 초안이다. 기존 600cm/s 이동을 유지했으며 재생 배속은 0.5~2배로 제한한다.
따라서 빠른 이동에서는 발이 미끄러져 보일 수 있다. 소서리스풍의 자연스러운 움직임으로
다듬으려면 보폭/발 접지/배속을 함께 조정해야 한다. 파이어볼과 일어나기도 손작업 폴리싱이
필요한 초기 동작이며 원작과 동일한 완성도의 재현을 의미하지 않는다.
