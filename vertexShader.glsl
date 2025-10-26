#version 330 core            // Minimal GL version support expected from the GPU

layout(location=0) in vec3 vPosition;
layout(location=1) in vec3 vNormal;


uniform mat4 modelMat;
uniform mat4 viewMat, projMat;
uniform vec3 lColor; // passed from CPU per-mesh

out vec3 fColor;

void main() {
        // Transform vertex position by model, view, then projection matrices
        gl_Position = projMat * viewMat * modelMat * vec4(vPosition, 1.0);

        // Compute a simple lighting term using the normal (model space -> world space via modelMat)
        // normal matrix: inverse-transpose of the model's upper-left 3x3
        mat3 normalMat = transpose(inverse(mat3(modelMat)));
        vec3 n = normalize(normalMat * vNormal);
        // simple directional light coming roughly from the camera direction
        vec3 lightDir = normalize(vec3(0.0, 0.0, 1.0));
        float diff = max(dot(n, lightDir), 0.0);
        float ambient = 0.2;
        fColor = lColor * (ambient + (1.0 - ambient) * diff);
}

