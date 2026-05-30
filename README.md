# LostArk Clone

## 프로젝트 소개
로스트아크의 카오스 던전 모작 프로젝트입니다

## 개발 환경
- Unreal Engine 5.4
- Visual Studio 2022
- C++

## 주요 시스템
- 우클릭 이동
- 좌클릭 시, 크리스탈 파괴 및 몬스터 공격
- 몬스터의 플레이어 추적 
- 크리스탈에 의한 몬스터 소환 시스템
- 노말 몬스터와 레어 몬스터의 구분
- 레어 몬스터 사망 시, 클리어 시스템

## 구현 예정 시스템
- 플레이어 애님 클래스 작성
- 피격 시스템과 사망 시스템
- 플레이어 사망 시스템
- 재시작 시스템

## 실행 방법
1. `LostArk.uproject` 실행
2. 필요 시 Visual Studio에서 `Development Editor` 빌드

## 임시
- Attack 은 모션 없이 전부 왼쪽 클릭 한번으로 처리

## 폴더 구조
## Project Structure

LostArk/
├── Source/
│   └── LostArk/
│       ├── Player/               # Player Character / Controller
│       ├── Monster/              # Monster AI / Crystal / Spawn Logic
│       └── Public / Private      # Unreal C++ Structure
│
├── Content/
│   ├── Blueprints/
│   │   ├── Player/
│   │   └── Monster/
│   │
│   ├── TopDown/
│   │   ├── Maps/
│   │   └── Input/
│   │
│   ├── ROG_Creatures/        # Monster Assets
│   ├── QuadrapedCreatures/   # Creature Assets
│   └── Realistic_Starter_VFX_Pack_Vol2/
│
├── Config/
├── Plugins/
└── LostArk.uproject
## 추가 콘텐츠 팩
- ROG Creatures: Stickman
- Qudraped Fantasy Creatures / Centaur
- Realistic Starter VFX Pack Vol 2
