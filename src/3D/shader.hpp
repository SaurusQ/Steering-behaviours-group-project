#ifndef SHADER_HPP
#define SHADER_HPP

#include <glad/gl.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
  
class Shader
{
    public:
        // constructor reads and builds the shader
        Shader(const GLchar* vertexPath, const GLchar* fragmentPath);
        // use/activate the shader
        void use();
        // utility uniform functions
        void set(const std::string &name, const bool a) const;  
        void set(const std::string &name, const int a) const;   
        void set(const std::string &name, const float a) const;
        void setVec3(const std::string &name, const float *a, unsigned int number = 1);
        void setVec4(const std::string &name, const float *a, unsigned int number = 1);
        void setMat4(const std::string &name, const float *a, unsigned int number = 1);
        unsigned int getProgramID();
    private:
        void checkCompileErrors(unsigned int shader, std::string type);
        
        unsigned int ID_; //the program ID
};
  
#endif
