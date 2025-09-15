#version 330 core

out vec4 FragColor; // 현재 픽셀의 최종 결과

// 현재 렌더링 영역의 해상도
uniform vec2 u_resolution;

// 중심이 원점이고, 반지름이 r인 원에 대한 SDF
float SphereSDF(vec3 p, float r) {
	return length(p) - r;
}

// SDF 정의된 씬의 Surface Normal 구하기
vec3 EstimateNormal(vec3 p) {
	float EPSILON = 0.001;

	return normalize(vec3(
		SphereSDF(vec3(p.x + EPSILON, p.y, p.z), 1.0) - SphereSDF(vec3(p.x - EPSILON, p.y, p.z), 1.0),
		SphereSDF(vec3(p.x, p.y + EPSILON, p.z), 1.0) - SphereSDF(vec3(p.x, p.y - EPSILON, p.z), 1.0),
		SphereSDF(vec3(p.x, p.y, p.z + EPSILON), 1.0) - SphereSDF(vec3(p.x, p.y, p.z - EPSILON), 1.0)
	));
}

vec3 CalcPhongModel(vec3 rayPos, vec3 rayOrigin, vec3 normal, vec3 lightPos, vec3 lightColor, vec3 objectColor) {
	// Ambient
	float ambientStrength = 0.1;
	float ambient = ambientStrength;

	// Diffuse
	vec3 toLightDir = normalize(lightPos - rayPos); // 충돌 지점에서 광원으로 향함
	float diffuse = max(dot(normal, toLightDir), 0);

	// Specular
	float specularStrength = 0.5;
	vec3 viewDir = normalize(rayOrigin - rayPos);
	vec3 reflectedDir = reflect(-toLightDir, normal);
	float specular = pow(max(dot(viewDir, reflectedDir), 0.0), 32.0) * specularStrength;

	return (ambient + diffuse + specular) * lightColor * objectColor;
}

// 쉐이더 코드는 각 픽셀마다 독립적으로 실행됨
void main() {
	float EPSILON = 0.001;

	// 스크린 좌표 공간 -> UV 좌표 공간
	// gl_FragCoord
	// GLSL 내장 변수. 현재 셰이더가 계산하는 픽셀의 화면상 (x, y)좌표
	// 픽셀마다 다른 값을 가짐
	vec2 uv = (2.0 * gl_FragCoord.xy - u_resolution.xy) / u_resolution.y;

	// 카메라 설정
	vec3 rayOrigin = vec3(0.0, 0.0, -3.0);
	vec3 rayDir = normalize(vec3(uv, 1.0)); // 각 픽셀로 감
	vec3 rayPos = rayOrigin; // 현재 광선 위치

	// 광원 설정
	vec3 lightPos = vec3(3.0, 3.0, -3.0);	
	vec3 lightColor = vec3(1.0, 1.0, 1.0);

	// 물체 설정
	vec3 objectColor = vec3(0.58, 0.18, 0.85); // Violet

	// Ray Marching 알고리즘 구현
	float MAX_MARCHING_STEPS = 100;
	float MAX_MARCHING_DEPTH = 100;
	for (int i = 0; i < MAX_MARCHING_STEPS; i++) {
		float dist = SphereSDF(rayPos, 1.5);

		// 표면에 충분히 가까워졌다면 충돌로 판단
		if (dist < EPSILON) {
			// Phong Reflection Model 적용
			vec3 normal = EstimateNormal(rayPos);
			vec3 phongResult = CalcPhongModel(rayPos, rayOrigin, normal, lightPos, lightColor, objectColor);

			FragColor = vec4(phongResult, 1.0);
			return;
		}

		// 안전한 거리(SDF 값)만큼 광선 전진
		rayPos += rayDir * dist;

		// 너무 많이 갔으면 종료
		if (length(rayPos) > MAX_MARCHING_DEPTH) {
			break;
		}
	}

	FragColor = vec4(0.0, 0.0, 0.0, 1.0); // 충돌하지 못한 경우 검은색
	return;
}