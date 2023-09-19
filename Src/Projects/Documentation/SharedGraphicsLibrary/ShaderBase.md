# class ShaderBase

*Defined at C:/Users/Sergei/Documents/GitHub/OpenMoBu/Src/Projects/SharedGraphicsLibrary/ShaderBase.h#24*



 ShaderBase



**brief** A base class for gpu shaders, it's not ogl dependent  The class has some derives with ogl glsl implementation and test implementation

**see** GLSLShader



## Members

protected char[256] m_HeaderText



## Functions

### ShaderBase<T>

*public void ShaderBase<T>()*

*Defined at C:/Users/Sergei/Documents/GitHub/OpenMoBu/Src/Projects/SharedGraphicsLibrary/ShaderBase.cpp#8*

### ~ShaderBase<T>

*public void ~ShaderBase<T>()*

*Defined at C:/Users/Sergei/Documents/GitHub/OpenMoBu/Src/Projects/SharedGraphicsLibrary/ShaderBase.h#29*

 a destructor

### GetShaderType

*public const enum SharedGraphicsLibrary::ShaderType GetShaderType()*

 an abstract for derived implementation, to know an implementation type



**return** an implementation type (glsl, moc shader. etc.)

### SetHeaderText

*public void SetHeaderText(const char * text)*

*Defined at C:/Users/Sergei/Documents/GitHub/OpenMoBu/Src/Projects/SharedGraphicsLibrary/ShaderBase.cpp#14*

 resources

### LoadShaders

*public _Bool LoadShaders(const SharedMotionLibrary::CFileBase & vertex_file, const SharedMotionLibrary::CFileBase & fragment_file)*

 TODO: why shader have to manage any file system stuff ?!

### LoadShaders

*public _Bool LoadShaders(const T & referenceShader, const SharedMotionLibrary::CFileBase & fragment_file)*

 reuse vertex program from teh referenceShader

### ReCompileShaders

*public _Bool ReCompileShaders(const SharedMotionLibrary::CFileBase & vertex_file, const SharedMotionLibrary::CFileBase & fragment_file)*

 TODO: do we want to recompile optional one of shaders ?!

### Bind

*public T & Bind()*

 bind a shader, have to be accompine with UnBind call

### UnBind

*public T & UnBind()*

### SetUniformUINT

*public _Bool SetUniformUINT(const char * name, const int value)*

### SetUniformFloat

*public _Bool SetUniformFloat(const char * name, const float value)*

### SetUniformVector

*public _Bool SetUniformVector(const char * name, const float x, const float y, const float z, const float w)*

### SetUniformVector2f

*public _Bool SetUniformVector2f(const char * name, const float x, const float y)*

### SetUniformMatrix33

*public _Bool SetUniformMatrix33(const char * name, const float * m)*

### SetUniformMatrix

*public _Bool SetUniformMatrix(const char * name, const float * m)*

### FindLocation

*public int FindLocation(const char * name)*

### SetUniformUINT

*public T & SetUniformUINT(const int location, const int value)*

### SetUniformFloat

*public T & SetUniformFloat(const int location, const float value)*

### SetUniformVector

*public T & SetUniformVector(const int location, const float x, const float y, const float z, const float w)*

### SetUniformVector4

*public T & SetUniformVector4(const int location, const float * arr)*

### SetUniformVector2f

*public T & SetUniformVector2f(const int location, const float x, const float y)*

### SetUniformMatrix33

*public T & SetUniformMatrix33(const int location, const float * m)*

### SetUniformMatrix

*public T & SetUniformMatrix(const int location, const float * m)*

### BindTexture

*public T & BindTexture(unsigned int target, const char * texname, unsigned int texid, int texunit)*

### BindTexture

*public T & BindTexture(unsigned int target, const int location, unsigned int texid, int texunit)*

### UnBindTexture

*public T & UnBindTexture(unsigned int target, int texunit)*

### BindTextureRECT

*public T & BindTextureRECT(const char * texname, unsigned int texid, int texunit)*

### GetHeaderLength

*public int GetHeaderLength()*

*Defined at C:/Users/Sergei/Documents/GitHub/OpenMoBu/Src/Projects/SharedGraphicsLibrary/ShaderBase.cpp#21*

### GetHeaderCapacity

*protected const int GetHeaderCapacity()*

*Defined at C:/Users/Sergei/Documents/GitHub/OpenMoBu/Src/Projects/SharedGraphicsLibrary/ShaderBase.h#91*

### GetHeaderText

*protected const char * GetHeaderText()*

*Defined at C:/Users/Sergei/Documents/GitHub/OpenMoBu/Src/Projects/SharedGraphicsLibrary/ShaderBase.h#92*



