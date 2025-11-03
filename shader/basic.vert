#version 330 core

// main.cpp에서 location = 0으로 보낸 정점 위치와
// u_mvp라는 이름으로 보내준 MVP 행렬 받아서
// 각 정점의 최종 화면 좌표(gl_position) 결정

// glVertexAttribPointer에서 location = 0으로 지정한 정점 위치 데이터 받음
layout (location = 0) in vec3 aPos;

// glUniformMatrix4fv로 전달한 MVP 행렬
uniform mat4 u_mvp;

void main()
{
	// 정점 위치(aPos)에 MVP 행렬 곱해 최종 공간 좌표 계산 후
	// gl_Position에 할당
	gl_Position = u_mvp * vec4(aPos, 1.0);
}