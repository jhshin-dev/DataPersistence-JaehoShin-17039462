# PRD: DataPersistence PoC

## 목적

반도체 시료 생산주문관리 시스템(S-Semi) 본 개발 착수 전, **JSON 파일 기반의 데이터 영속성 구조**가 실제로 동작하는지 검증한다.  
이 PoC가 통과되면 동일한 Repository 인터페이스를 유지한 채 저장 백엔드(SQLite 등)를 교체하거나 전체 시스템을 확장할 수 있다.

---

## 범위

| 구분 | 포함 여부 |
|---|---|
| Sample CRUD (파일 저장/불러오기) | ✅ |
| Order CRUD (파일 저장/불러오기) | ✅ |
| 주문 상태 전이 로직 | ✅ (status 필드 업데이트만) |
| 콘솔 UI / 역할별 메뉴 | ❌ |
| 생산 라인 시뮬레이션 | ❌ |
| 인증 / 권한 관리 | ❌ |
| 네트워크 통신 | ❌ |

---

## 도메인 모델

### Sample

| 필드 | 타입 | 설명 |
|---|---|---|
| `id` | `int` | 저장소가 자동 부여하는 고유 식별자 |
| `name` | `string` | 시료 이름 (예: "GaN-001") |
| `spec` | `string` | 시료 사양 (예: 두께, 도핑 농도 등 자유 문자열) |
| `stock` | `int` | 현재 보유 재고 수량 |

### Order

| 필드 | 타입 | 설명 |
|---|---|---|
| `id` | `int` | 저장소가 자동 부여하는 고유 식별자 |
| `sampleId` | `int` | 주문 대상 Sample의 id |
| `quantity` | `int` | 요청 수량 |
| `status` | `OrderStatus` | 현재 주문 상태 |
| `createdAt` | `string` | ISO 8601 생성 시각 |
| `updatedAt` | `string` | ISO 8601 최종 수정 시각 |

### OrderStatus

```
RESERVED   주문 접수 (초기 상태)
REJECTED   주문 거절 (단말 상태 — 정상 흐름 제외)
PRODUCING  승인 완료, 재고 부족으로 생산 중
CONFIRMED  출고 대기 (재고 충분 or 생산 완료)
RELEASED   출고 완료 (단말 상태)
```

상태 전이 규칙:

```
RESERVED  → CONFIRMED  (승인 + 재고 충분)
RESERVED  → PRODUCING  (승인 + 재고 부족)
RESERVED  → REJECTED   (거절)
PRODUCING → CONFIRMED  (생산 완료)
CONFIRMED → RELEASED   (출고 처리)
```

그 외 전이는 허용하지 않는다. 잘못된 전이 시도는 에러를 반환하고 데이터를 변경하지 않는다.

---

## 기능 요구사항

### F-01: SampleRepository CRUD

| ID | 요구사항 |
|---|---|
| F-01-1 | Sample을 생성하고 `samples.json`에 저장한다. `id`는 저장소가 자동 부여한다. |
| F-01-2 | 전체 Sample 목록을 `samples.json`에서 읽어 반환한다. |
| F-01-3 | `id`로 단일 Sample을 조회한다. 존재하지 않으면 `std::nullopt`를 반환한다. |
| F-01-4 | Sample의 `name`, `spec`, `stock` 필드를 수정하고 파일에 반영한다. |
| F-01-5 | `id`로 Sample을 삭제한다. 해당 Sample을 참조하는 Order가 존재하면 삭제를 거부한다. |

### F-02: OrderRepository CRUD

| ID | 요구사항 |
|---|---|
| F-02-1 | Order를 생성한다. 초기 `status`는 항상 `RESERVED`이고, `createdAt`/`updatedAt`을 현재 시각으로 기록한다. |
| F-02-2 | 전체 Order 목록을 `orders.json`에서 읽어 반환한다. |
| F-02-3 | `id`로 단일 Order를 조회한다. 존재하지 않으면 `std::nullopt`를 반환한다. |
| F-02-4 | Order의 `status`를 변경한다. 허용된 전이가 아니면 실패한다. 성공 시 `updatedAt`을 갱신하고 파일에 반영한다. |
| F-02-5 | `id`로 Order를 삭제한다. `RELEASED` 상태의 Order는 삭제를 거부한다. |

### F-03: 영속성 보장

| ID | 요구사항 |
|---|---|
| F-03-1 | 쓰기 작업(Create / Update / Delete)은 즉시 파일에 반영한다 (load-on-open, save-on-write). |
| F-03-2 | 프로세스를 재시작해도 이전에 저장된 데이터가 그대로 복원된다. |
| F-03-3 | JSON 파일이 존재하지 않으면 빈 컬렉션으로 초기화하고 새 파일을 생성한다. |
| F-03-4 | JSON 파일이 파싱 불가능한 상태면 예외를 던지고 프로세스를 중단한다 (손상 파일 자동 복구 불필요). |

### F-04: 검증용 main 시나리오

PoC 성공 여부를 확인하기 위해 `main.cpp`에 아래 시나리오를 순서대로 실행하는 코드를 작성한다.

1. Sample 2개 생성 → 파일 저장 확인 (콘솔 출력)
2. 전체 Sample 조회 → 콘솔 출력
3. Order 1개 생성 (`RESERVED`) → 파일 저장 확인
4. Order 상태를 `CONFIRMED`으로 전이 → 파일 변경 확인
5. Order 상태를 `RELEASED`로 전이 → 파일 변경 확인
6. 프로세스 재시작 후 전체 데이터 재조회 → 동일 데이터 복원 확인

---

## 비기능 요구사항

| ID | 요구사항 |
|---|---|
| NF-01 | C++20 표준만 사용한다. OS API 직접 호출은 파일 I/O 이외에는 사용하지 않는다. |
| NF-02 | 외부 의존성은 `nlohmann/json` 단일 헤더(`json.hpp`)만 허용한다. vcpkg·CMake 불필요. |
| NF-03 | `Repository<T>` 인터페이스는 순수 가상 함수로 정의하여 추후 백엔드 교체가 가능한 구조를 유지한다. |
| NF-04 | 빌드 경고 수준 Level3(`/W3`)에서 경고 없이 컴파일되어야 한다. |

---

## 인수 기준

아래 조건을 모두 만족하면 PoC 완료로 판단한다.

- [ ] `msbuild` Debug x64 빌드가 경고 없이 성공한다.
- [ ] `main.cpp` 시나리오(F-04)가 에러 없이 끝까지 실행된다.
- [ ] 실행 후 `samples.json`과 `orders.json` 파일이 생성되고, 저장된 내용이 콘솔 출력과 일치한다.
- [ ] 프로세스를 재시작한 뒤 같은 데이터가 복원되는 것을 콘솔 출력으로 확인한다.
- [ ] 잘못된 상태 전이 시도 시 데이터가 변경되지 않고 에러 메시지가 출력된다.
