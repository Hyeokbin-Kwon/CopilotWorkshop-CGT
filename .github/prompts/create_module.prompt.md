---
mode: 'agent'
model: Claude Sonnet 4
tools: ['changes', 'codebase', 'githubRepo', 'runTests', 'testFailure', 'fetch', 'filesystem', 'Azure MCP Server', 'cpp']
description: "C모듈을 생성합니다."
---


# 새로운 모듈 생성

아래 정보를 바탕으로 C 모듈(헤더 파일과 소스 파일)을 생성해 주세요:

- 모듈 이름: {{module_name}}
- 기능 설명: {{description}}
- 의존성: {{dependencies}}

다음 코딩 스타일 규칙을 따라 주세요:
- 함수 이름은 snake_case 사용
- 적절한 주석 포함
- 에러 처리를 위해 성공 시 0, 실패 시 -1 반환

헤더 파일은 보호자(guard)를 포함해야 합니다.