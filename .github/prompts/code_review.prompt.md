---
mode: 'agent'
model: Claude Sonnet 4
tools: ['changes', 'codebase', 'githubRepo', 'runTests', 'testFailure', 'fetch', 'filesystem', 'Azure MCP Server', 'cpp']
description: "코드를 리팩토링 합니다."
---

# 코드 검토

선택한 C 코드를 검토하고 다음 항목을 확인해 주세요:

1. 메모리 누수 가능성
2. 버퍼 오버플로우 위험
3. 널 포인터 참조
4. SQL 인젝션 취약점
5. 함수 가독성 및 모듈화
6. 에러 처리 적절성
7. 코딩 스타일 준수

각 문제점에 대해 구체적인 수정 방법을 제안해 주세요.