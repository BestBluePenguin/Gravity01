#version 330 core
in vec4 objectColor;
out vec4 FragColor;

void main()
{
    FragColor = vec4(objectColor.rgb, objectColor.a);

}
