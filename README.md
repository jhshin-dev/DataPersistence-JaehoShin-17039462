# DataPersistence PoC

반도체 시료 생산주문관리 시스템 **S-Semi**의 데이터 영속성 PoC입니다.  
JSON 파일 기반 Repository 패턴으로 Sample·Order CRUD와 주문 상태 전이를 검증합니다.

## 빌드 및 실행

```powershell
# Debug x64 빌드
msbuild DataPersistence.slnx /p:Configuration=Debug /p:Platform=x64

# 실행
.\x64\Debug\DataPersistence.exe
```

> **환경**: Visual Studio 2022 (MSVC v145), C++20, Windows x64

## 구조

```
IRepository<T>          순수 가상 CRUD 인터페이스
└─ JsonRepository<T>    JSON 파일 기반 구현 (load-on-open / save-on-write)
   ├─ SampleRepository  참조 중인 Order가 있으면 삭제 거부
   └─ OrderRepository   상태 전이 유효성 검사 + 타임스탬프 자동 관리
```

외부 의존성: [`nlohmann/json`](https://github.com/nlohmann/json) (단일 헤더 `json.hpp`)

## 주문 상태 흐름

```
RESERVED ──► CONFIRMED ──► RELEASED
         └─► PRODUCING ──►
         └─► REJECTED
```

`RELEASED` · `REJECTED`는 단말 상태이며 이후 전이 불가.

## PoC 검증 결과

| 항목 | 결과 |
|---|---|
| /W3 경고 없이 빌드 | ✅ |
| Sample / Order CRUD | ✅ |
| 유효 상태 전이 (RESERVED→PRODUCING→CONFIRMED→RELEASED) | ✅ |
| 유효 상태 전이 (RESERVED→CONFIRMED→RELEASED) | ✅ |
| 유효 상태 전이 (RESERVED→REJECTED) | ✅ |
| 잘못된 전이 거부 (RESERVED→RELEASED 등) | ✅ |
| RELEASED 주문 삭제 거부 | ✅ |
| Order 참조 중인 Sample 삭제 거부 | ✅ |
| 재시작 후 데이터 복원 (ID 연속성 포함) | ✅ |
| 혼재 상태 데이터 (외부 주입) 정상 로드 | ✅ |
| 손상된 JSON 파싱 실패 시 예외 처리 | ✅ |
