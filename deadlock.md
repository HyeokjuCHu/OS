# Deadlock(교착 상태)

## Deadlock이란? (슬라이드 1-3)
Deadlock은 시스템이 교착 상태에 빠져 아무것도 할 수 없는 상황을 의미합니다. 컴퓨터 시스템에서는 여러 **스레드(Thread)**나 **프로세스(Process)**가 특정 **자원(Resource)**을 얻으려고 할 때 발생합니다.

> **Deadlock 정의**  
> A set of threads is in a deadlocked state when every thread in the set is waiting for an event that can be caused only by another thread in the set.

즉, 집합 안의 모든 스레드가 서로가 가진 자원을 기다리면서 영원히 대기하는 상태입니다. 주요 이벤트는 **자원 획득(resource acquisition)**과 **자원 방출(release)**입니다.

### Deadlock의 실제 예시 (슬라이드 4)
Deadlock이 발생하는 코드 예시는 다음과 같습니다:

- **자원**: `first_mutex`와 `second_mutex`
- **스레드**:
  - `thread_one`: `first_mutex`를 먼저 잠그고, 그 다음에 `second_mutex`를 잠그려고 함.
  - `thread_two`: `second_mutex`를 먼저 잠그고, 그 다음에 `first_mutex`를 잠그려고 함.

#### Deadlock 발생 시나리오:
1. `thread_one`이 `first_mutex`를 획득.
2. 동시에 `thread_two`가 `second_mutex`를 획득.
3. `thread_one`은 `second_mutex`를 기다리고, `thread_two`는 `first_mutex`를 기다림.
4. 둘 다 서로가 가진 자원을 놓아주기만을 영원히 기다리게 됨.

이 상황은 **Resource-allocation graph**로 시각화할 수 있습니다:
- `thread_one`은 `first_mutex`를 점유하고 `second_mutex`를 기다림.
- `thread_two`는 `second_mutex`를 점유하고 `first_mutex`를 기다림.
- 화살표가 원형으로 이어지며 Deadlock이 발생.

---

## Deadlock 발생의 4가지 조건 (슬라이드 5)
Deadlock은 아래 4가지 조건이 동시에 충족될 때 발생합니다:

1. **Mutual exclusion (상호 배제)**  
   자원은 한 번에 하나의 스레드만 사용할 수 있습니다.  
   예: "이 프린터는 지금 나만 쓸 거야!"

2. **Hold and wait (점유 및 대기)**  
   스레드가 최소 하나의 자원을 점유한 상태에서, 다른 스레드가 점유한 자원을 추가로 요청하며 기다립니다.

3. **No preemption (비선점)**  
   다른 스레드로부터 자원을 강제로 빼앗을 수 없습니다. 자원을 다 쓴 스레드가 스스로 놓아줄 때까지 기다려야 합니다.

4. **Circular wait (환형 대기)**  
   스레드들이 원형으로 서로의 자원을 기다리는 상황입니다.  
   예: \(T_0\)는 \(T_1\)의 자원을, \(T_1\)은 \(T_2\)의 자원을, ..., \(T_n\)은 \(T_0\)의 자원을 기다리는 꼬리물기 상태.

---

## Resource-Allocation Graph (자원 할당 그래프) (슬라이드 6-8)
Deadlock을 체계적으로 표현하기 위한 도구입니다.

### 그래프 구성 요소:
1. **정점(Vertices)**  
   - **스레드(Threads)**: \(T = \{T_1, T_2, \dots, T_n\}\) (원으로 표현)  
   - **자원 유형(Resource types)**: \(R = \{R_1, R_2, \dots, R_m\}\) (사각형으로 표현). 사각형 안의 점은 해당 자원의 인스턴스(개수)를 의미.

2. **간선(Edges)**  
   - **Request edge (요청 간선)**: \(T_i \rightarrow R_j\). 스레드 \(T_i\)가 자원 \(R_j\)를 요청 중임을 의미.  
   - **Assignment edge (할당 간선)**: \(R_j \rightarrow T_i\). 자원 \(R_j\)가 스레드 \(T_i\)에 할당되었음을 의미.

### Deadlock 여부 판단:
- 그래프에 **Cycle(순환)**이 없으면 Deadlock도 없음.
- 그래프에 **Cycle**이 있으면 Deadlock일 가능성이 있음.
  - 자원 유형별 인스턴스가 1개뿐이라면, Cycle은 반드시 Deadlock을 의미.
  - 자원 유형별 인스턴스가 여러 개라면, Cycle이 있어도 Deadlock이 아닐 수 있음.

---

## Deadlock 처리 방법 (슬라이드 9-10)
Deadlock을 처리하는 방법은 크게 세 가지로 나뉩니다:

1. **문제를 무시하기**  
   - "타조 알고리즘"이라고도 불림. Deadlock은 매우 드물게 발생한다고 가정하고 아무런 조치도 하지 않음.  
   - 대부분의 범용 운영체제(Windows, UNIX)가 이 방식을 사용. 문제가 생기면 사용자가 재부팅.

2. **Deadlock Prevention (예방)** 또는 **Avoidance (회피)**  
   - Deadlock이 발생할 수 있는 상황 자체를 만들지 않음.  
   - Prevention: Deadlock 발생 조건 중 하나를 원천적으로 깨뜨림.  
   - Avoidance: 자원 할당 시 Deadlock 가능성을 검사하여 안전할 때만 할당.

3. **Deadlock Detection (탐지) 및 Recovery (회복)**  
   - Deadlock이 발생하도록 내버려 둔 뒤, 이를 탐지하고 해결.

---

## Deadlock Prevention (예방) (슬라이드 11-14)
Deadlock 발생의 4가지 조건 중 하나를 깨뜨리는 방법입니다.

1. **Mutual Exclusion 깨기**  
   공유 가능한 자원(e.g., read-only 파일)에 대해서는 상호 배제를 적용하지 않음.  
   하지만 프린터처럼 본질적으로 공유가 불가능한 자원이 많아 현실적으로 어렵습니다.

2. **Hold and Wait 깨기**  
   - 스레드가 자원을 요청할 때는 다른 자원을 가지고 있지 않도록 보장.  
   - 실행 시작 전에 필요한 모든 자원을 한꺼번에 할당받거나, 자원이 필요하면 가진 자원을 모두 반납하고 다시 요청.

3. **No Preemption 깨기**  
   - 자원을 선점(preempt)할 수 있게 만듦.  
   - 자원을 요청했는데 바로 할당받을 수 없다면, 현재 가지고 있는 자원을 모두 빼앗아 대기 목록에 넣음.

4. **Circular Wait 깨기**  
   - 모든 자원 유형에 고유한 번호를 부여하고, 스레드가 번호가 증가하는 순서로만 자원을 요청하도록 강제.  
   - 예: `F(first_mutex)=1`, `F(second_mutex)=5`. 모든 스레드가 동일한 순서로 자원을 요청하게 되어 순환 대기가 발생하지 않음.

---

## Deadlock Avoidance (회피) (슬라이드 15-26)
시스템이 **Safe State(안전 상태)**를 유지하도록 자원 할당을 관리하는 방법입니다.

### Safe State
- 시스템에 있는 모든 스레드가 Deadlock 없이 정상적으로 종료될 수 있는 **실행 순서(Safe sequence)**가 최소 하나 이상 존재하는 상태.

### Avoidance 알고리즘
1. **Resource-Allocation Graph Algorithm** (자원 인스턴스가 1개일 때)  
   - Cycle이 생기지 않도록 자원 요청을 관리.

2. **Banker's Algorithm** (자원 인스턴스가 여러 개일 때)  
   - 스레드가 시작 전에 최대로 사용할 자원의 양을 미리 선언.  
   - 자원 요청 시 시스템이 Safe State를 유지하는지 확인.

---

## Deadlock Detection (탐지) & Recovery (회복) (슬라이드 27-35, 37-41)
Deadlock이 발생하는 것을 허용한 뒤, 이를 탐지하고 해결하는 방식입니다.

### Deadlock Detection
1. **Single Instance of Each Resource Type** (자원 인스턴스가 1개일 때)  
   - **Wait-for graph**를 사용하여 Cycle을 탐지.

2. **Multiple Instances of a Resource Type** (자원 인스턴스가 여러 개일 때)  
   - Banker's Algorithm과 유사한 탐지 알고리즘 사용.

### Recovery from Deadlock
1. **Process Termination**  
   - Deadlock에 걸린 모든 프로세스를 강제 종료.  
   - Deadlock이 풀릴 때까지 한 번에 하나씩 프로세스를 종료.

2. **Resource Preemption**  
   - 희생양 프로세스를 골라 자원을 빼앗음.  
   - 특정 프로세스가 계속 희생양으로 선택되지 않도록 주의.

---

## 결론
Deadlock은 운영체제에서 반드시 고려해야 할 중요한 문제입니다. 이를 해결하기 위해 **Prevention**, **Avoidance**, **Detection & Recovery**의 세 가지 접근법이 사용됩니다.

- **Prevention**: Deadlock 발생 조건을 원천적으로 깨뜨림.
- **Avoidance**: 자원 할당 시 안전성을 검사하여 유연하게 대처.
- **Detection & Recovery**: Deadlock 발생을 허용한 뒤 이를 탐지하고 해결.

시스템의 특성과 요구사항에 따라 적절한 방법을 선택하거나 조합하여 사용해야 합니다.