#version 330 core
out vec4 FragColor;

uniform vec4 boundColor;

void main()
{
    //Flat color
    FragColor = boundColor;
}