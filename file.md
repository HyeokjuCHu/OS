# 파일 시스템 (File Systems) 상세 정리

---

## I. Files and Directories (파일과 디렉터리)

### 1. 기본 개념 (Concepts)
* [cite_start]**File**: 바이트(byte)의 선형 배열(linear array of bytes)입니다. [cite_start]각 파일은 `inode number`라는 저수준(low-level) 이름을 가집니다.
* [cite_start]**Directory**: 디렉터리 또한 파일의 한 종류로, 사용자가 읽을 수 있는 파일명(user-readable filename)과 저수준 파일명(`inode number`)의 쌍으로 이루어진 목록을 담고 있습니다. [cite_start]파일 시스템은 루트 디렉터리(`/`)를 최상위로 하는 트리 구조를 형성합니다.

### 2. 파일 및 디렉터리 조작 APIs

#### 파일 조작 API (File Manipulation APIs)
* [cite_start]`open()`: 파일을 열거나 생성합니다.
* [cite_start]`close()`: 열려 있는 파일 디스크립터(file descriptor)를 닫습니다.
* [cite_start]`read()`: 파일에서 데이터를 읽습니다.
* [cite_start]`write()`: 파일에 데이터를 씁니다.
* [cite_start]`lseek()`: 파일 오프셋(file offset)을 이동시킵니다.
* [cite_start]`unlink()`: 파일을 삭제합니다.
* [cite_start]`link()`: 파일에 대한 하드 링크(hard link)를 생성합니다.
* [cite_start]`symlink()`: 심볼릭 링크(symbolic link)를 생성합니다.
* [cite_start]`stat()`, `fstat()`, `lstat()`: 파일의 메타데이터(metadata)를 가져옵니다.
* [cite_start]`fsync()`: 버퍼에 있는 변경 사항을 디스크에 강제로 저장합니다.
* [cite_start]`chown()`, `fchown()`: 파일의 소유권을 변경합니다.
* [cite_start]`chmod()`, `fchmod()`: 파일의 권한을 변경합니다.

#### 디렉터리 조작 API (Directory Manipulation APIs)
* [cite_start]`mkdir()`: 새로운 디렉터리를 생성합니다.
* [cite_start]`rmdir()`: 비어있는 디렉터리를 삭제합니다.
* [cite_start]`opendir()`: 디렉터리를 엽니다 (`DIR*` 포인터 반환).
* [cite_start]`readdir()`: 디렉터리 내의 항목들을 순차적으로 읽습니다.
* [cite_start]`closedir()`: 열려 있는 디렉터리를 닫습니다.
* [cite_start]`getcwd()`: 현재 작업 디렉터리의 경로를 가져옵니다.

### 3. 주요 개념 상세 설명

#### Open File Table (열린 파일 테이블)
* [cite_start]자식 프로세스는 부모 프로세스의 파일 디스크립터 테이블을 상속받습니다.
* [cite_start]`fork()` 호출 시, 부모와 자식 프로세스의 파일 디스크립터는 시스템 전역의 `Open File Table`에 있는 동일한 항목을 가리킵니다.
* [cite_start]이 테이블 항목은 파일 오프셋(offset)과 같은 정보를 공유하므로, 한 프로세스에서 `lseek()`로 오프셋을 변경하면 다른 프로세스에도 그 변경 사항이 반영됩니다.

#### `fsync()`와 영속성 (Persistency)
* [cite_start]`write()` 함수는 데이터를 즉시 디스크에 저장하지 않고 커널의 버퍼 캐시에 쓸 수 있습니다.
* [cite_start]데이터베이스(DBMS)처럼 데이터의 즉각적인 저장이 보장되어야 하는 애플리케이션에서는 `fsync()`를 호출하여 버퍼의 내용을 디스크에 강제로 동기화(저장)해야 합니다.

#### `stat()`와 파일 상태 정보
* [cite_start]`stat()` 계열 함수들은 파일의 상태 정보(메타데이터)를 조회합니다.
    * [cite_start]`stat()`: 경로명(pathname)으로 파일 정보를 가져옵니다.
    * [cite_start]`fstat()`: 파일 디스크립터(fd)로 파일 정보를 가져옵니다.
    * [cite_start]`lstat()`: 경로가 심볼릭 링크일 경우, 링크 자체의 정보를 반환합니다.
* [cite_start]조회된 정보는 `struct stat` 구조체에 저장되며, 이 구조체는 다음 정보를 포함합니다:
    * [cite_start]`st_dev`: 장치 ID 
    * [cite_start]`st_ino`: Inode 번호 
    * [cite_start]`st_mode`: 파일 타입 및 모드(권한) 
    * [cite_start]`st_nlink`: 하드 링크의 수 
    * [cite_start]`st_uid`: 소유자 사용자 ID 
    * [cite_start]`st_gid`: 소유자 그룹 ID 
    * [cite_start]`st_size`: 파일 크기 (bytes) 
    * [cite_start]`st_blocks`: 할당된 블록 수 
    * [cite_start]`st_atime`, `st_mtime`, `st_ctime`: 최근 접근, 수정, 상태 변경 시간 
* [cite_start]`st_mode` 필드의 비트마스크(`S_IFMT`)를 사용해 파일 타입을 확인할 수 있으며(`S_IFREG`는 일반 파일, `S_IFDIR`은 디렉터리 등) [cite: 29, 30][cite_start], `S_ISREG()`와 같은 매크로를 사용하는 것이 더 편리합니다. [cite_start]또한 파일 접근 권한(permission) 정보도 포함되어 있습니다.

#### `opendir()` & `readdir()`
* [cite_start]디렉터리는 특별한 구조를 가진 파일이므로, 일반 `read()`가 아닌 `opendir()`, `readdir()`를 사용해 내용을 읽어야 합니다.
* [cite_start]`opendir()`는 디렉터리 스트림(`DIR*`)을 열고 [cite: 37][cite_start], `readdir()`는 이 스트림에서 `struct dirent` 구조체 형태로 각 항목(파일 또는 하위 디렉터리)을 하나씩 읽어옵니다.
* [cite_start]`struct dirent` 구조체는 `inode` 번호(`d_ino`)와 파일 이름(`d_name`) 등의 정보를 담고 있습니다.

#### `link()` (Hard Link)와 `unlink()`
* [cite_start]`link()`는 기존 파일에 대한 새로운 이름, 즉 하드 링크를 생성하는 시스템 콜입니다.
* [cite_start]하드 링크로 연결된 파일들은 동일한 `inode` 번호를 공유하며, 원본과 링크는 완전히 동일하게 취급됩니다. [cite_start]`inode`는 자신을 가리키는 링크의 수를 `link count`로 관리합니다.
* [cite_start]`unlink()`는 파일의 이름(링크)을 파일 시스템에서 삭제합니다 (`rm` 명령어는 내부적으로 이 함수를 호출합니다).
* [cite_start]`unlink()` 호출 시 `inode`의 참조 카운트가 1 감소합니다. [cite_start]이 카운트가 0이 되고, 어떤 프로세스도 해당 파일을 열고 있지 않을 때 비로소 파일 데이터가 디스크에서 완전히 삭제됩니다.

#### Symbolic Links (심볼릭 링크)
* [cite_start]심볼릭 링크(소프트 링크)는 원본 파일이나 디렉터리의 경로를 내용으로 갖는 특별한 타입의 파일입니다.
* 하드 링크와의 주요 차이점:
    * [cite_start]자신만의 고유한 `inode`를 가집니다.
    * [cite_start]디렉터리에 대한 링크를 생성할 수 있습니다.
    * [cite_start]다른 파일 시스템 파티션에 있는 파일을 가리킬 수 있습니다.
    * [cite_start]원본 파일이 삭제되면 링크가 깨지는 '댕글링 참조(dangling reference)'가 발생할 수 있습니다.

---

## II. File System Structure & Operations (파일 시스템 구조 및 연산)

### 1. 저장 장치 및 주소 지정
* [cite_start]현대 컴퓨터의 주된 보조 저장 장치는 `HDD(Hard Disk Drives)`와 `NVM(Nonvolatile Memory)` 장치(예: SSD)입니다.
* [cite_start]디스크 드라이브는 `logical blocks`의 1차원 배열로 주소화됩니다. [cite_start]이는 물리적인 주소(실린더, 헤드, 섹터 등)를 추상화하여 사용을 간편하게 합니다.

### 2. 디스크 구조 (Disk Structure)
* [cite_start]디스크는 여러 개의 파티션(`partitions`)으로 나뉠 수 있습니다.
* [cite_start]파일 시스템을 포함하는 단위를 볼륨(`volume`)이라고 하며, 각 볼륨은 파일 시스템 정보를 담고 있는 `device directory`를 가집니다.

### 3. 파일 시스템 마운트 (File System Mounting)
* [cite_start]파일 시스템에 접근하려면 반드시 기존 디렉터리 트리의 특정 지점, 즉 마운트 지점(`mount point`)에 마운트(`mount`)되어야 합니다.

### 4. 파일 시스템 계층 구조 (Layered Structure)
[cite_start]파일 시스템은 복잡성을 줄이기 위해 여러 계층으로 구성됩니다.
1.  [cite_start]**Application Programs (응용 프로그램)** 
2.  [cite_start]**Logical File System (논리 파일 시스템)**: 파일 이름과 `inode` 번호 관리, 파일 제어 블록(FCB)을 다룹니다.
3.  [cite_start]**File-Organization Module (파일 조직 모듈)**: 파일의 논리적 블록과 물리적 블록 간 매핑을 담당합니다.
4.  [cite_start]**Basic File System (기본 파일 시스템)**: 저장 장치에 보낼 I/O 요청을 생성합니다.
5.  [cite_start]**I/O Control (입출력 제어)**: 장치 드라이버와 인터럽트 핸들러를 포함합니다.
6.  [cite_start]**Devices (장치)**: 실제 물리적 저장 장치입니다.

### 5. 파일 시스템 자료 구조
* **디스크 내 구조 (On-Disk Structures)**
    * [cite_start]`Boot Control Block`: 해당 볼륨에서 OS 부팅에 필요한 정보를 담습니다.
    * [cite_start]`Volume Control Block` (`superblock`): 볼륨 전체의 상세 정보(총 블록 수, 빈 블록 수, 블록 크기 등)를 포함합니다.
    * [cite_start]`Directory Structure`: 파일 이름과 해당 `inode` 번호를 연결하여 파일들을 조직화합니다.
* **메모리 내 구조 (In-Memory Structures)**
    * [cite_start]`Mount Table`: 마운트된 파일 시스템들의 정보를 저장합니다.
    * [cite_start]`System-wide Open-file Table`: 열려 있는 모든 파일의 FCB(File Control Block) 사본을 유지합니다.
    * [cite_start]`Per-process Open-file Table`: 각 프로세스마다 열려 있는 파일 목록을 관리하며, 시스템 전역 테이블의 항목을 가리키는 포인터를 포함합니다.

---

## III. File System Implementation (파일 시스템 구현)

### 1. 전체적인 디스크 구성
파일 시스템은 디스크 공간을 다음과 같은 영역으로 나누어 구성합니다.
* [cite_start]**Blocks**: 디스크를 일정한 크기(예: 4KB)의 블록으로 분할합니다.
* [cite_start]**Data Region**: 대부분의 공간은 사용자 데이터(파일 내용)를 저장하기 위한 `Data Region`으로 할당됩니다.
* [cite_start]**Inode Table**: 파일의 메타데이터를 담고 있는 `inode`들의 배열을 저장하기 위한 공간입니다. [cite_start]이 테이블의 크기가 파일 시스템이 가질 수 있는 최대 파일 수를 결정합니다.
* [cite_start]**Allocation Structures**: 데이터 블록과 `inode`가 현재 사용 중인지 아닌지를 추적하기 위해 `bitmap`을 사용합니다 (`inode bitmap`, `data bitmap`).
* [cite_start]**Superblock**: 파일 시스템 전체에 대한 정보(총 `inode` 수, `inode` 테이블의 시작 위치 등)를 담고 있습니다. [cite_start]파일 시스템 마운트 시 운영체제는 가장 먼저 `superblock`을 읽어 시스템을 초기화합니다.

### 2. Inode 상세
* [cite_start]**Inode 위치 계산**: 파일 시스템은 `inode number`를 이용해 `inode`가 디스크의 어느 위치에 있는지 계산할 수 있습니다.
* **Indexed Allocation (인덱스 할당)**: `inode`는 파일 데이터가 저장된 데이터 블록들의 주소를 가리키는 포인터들을 포함합니다. [cite_start]효율적인 공간 활용을 위해 여러 단계의 포인터 구조를 사용합니다.
    * [cite_start]`Direct blocks`: 데이터 블록을 직접 가리키는 포인터입니다.
    * [cite_start]`Single indirect block`: 데이터 블록 포인터들의 목록을 담고 있는 블록을 가리키는 포인터입니다.
    * [cite_start]`Double indirect block`: `single indirect block` 포인터들의 목록을 담고 있는 블록을 가리키는 포인터입니다.
    * [cite_start]`Triple indirect block`: 매우 큰 파일을 지원하기 위한 3단계 간접 포인터입니다.

### 3. Directory Structure
* [cite_start]디렉터리는 `inode number`와 파일 이름(`name`)을 매핑하는 항목들의 목록을 가진 데이터 블록입니다.
* [cite_start]`ext4` 파일 시스템의 경우 각 디렉터리 항목에 `inode` 번호, 레코드 길이(`rec_len`), 이름 길이(`name_len`), 파일 타입(`file_type`), 그리고 파일 이름(`name`)이 포함됩니다.

### 4. 파일 접근 경로 (Access Path)
* [cite_start]**파일 읽기 (Reading a File)**: `/foo/bar` 파일을 읽기 위해서는 루트(`/`) `inode`에서 시작하여 `foo` 디렉터리의 `inode`를 찾고, 다시 `foo`의 데이터에서 `bar`의 `inode`를 찾은 후, 최종적으로 `bar`의 `inode`에 명시된 데이터 블록들을 읽는 순차적인 I/O 과정이 필요합니다.
* **파일 쓰기 (Writing a File)**: 새로운 파일을 쓰는 과정은 더 복잡합니다. [cite_start]`bitmap`을 읽고 수정하여 빈 `inode`와 데이터 블록을 할당받고, 상위 디렉터리의 데이터에 새 파일 항목을 추가하며, 할당받은 `inode`와 데이터 블록에 내용을 기록하는 등 여러 쓰기 작업이 수반됩니다.

### 5. 캐싱과 버퍼링 (Caching and Buffering)
* [cite_start]느린 디스크 I/O 속도를 보완하기 위해 메모리 캐싱은 필수적입니다.
* [cite_start]**Buffer Cache**: 과거에는 파일 시스템 I/O만을 위한 고정된 크기의 메모리 영역(`Buffer Cache`)을 정적으로 할당하여 사용했습니다. [cite_start]이는 메모리 사용이 비효율적일 수 있습니다.
* [cite_start]**Page Cache**: 현대 운영체제는 가상 메모리(`virtual memory`) 시스템과 파일 I/O 캐시를 통합한 `Page Cache`를 사용합니다.
    * [cite_start]하나의 물리 메모리 페이지 프레임이 프로세스의 페이지 또는 파일의 데이터 블록을 저장하는 데 동적으로 사용될 수 있습니다.
    * [cite_start]이를 통해 시스템의 메모리 상황에 따라 프로세스와 파일 캐시 간의 메모리 할당을 유연하게 조절하여 전반적인 시스템 성능과 메모리 효율성을 높입니다.