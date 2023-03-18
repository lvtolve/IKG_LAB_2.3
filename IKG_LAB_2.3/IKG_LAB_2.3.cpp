#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "pipeline.h"

#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 768

GLuint VBO;// Объявление переменной для хранения идентификатора буфера вершин
GLuint IBO;// Объявление переменной для хранения идентификатора индексного буфера
GLuint gWVPLocation;// Объявление переменной для хранения местоположения переменной шейдера

static const char* pVS = "                                                          \n\
#version 330                                                                        \n\
layout (location = 0) in vec3 Position;                                             \n\
uniform mat4 gWVP;                                                                  \n\
out vec4 Color;                                                                     \n\
void main()                                                                         \n\
{                                                                                   \n\
    gl_Position = gWVP * vec4(Position, 1.0);                                       \n\
    Color = vec4(clamp(Position, 0.0, 1.0), 1.0);                                   \n\
}";

static const char* pFS = "                                                          \n\
#version 330                                                                        \n\
in vec4 Color;                                                                      \n\
out vec4 FragColor;                                                                 \n\
void main()                                                                         \n\
{                                                                                   \n\
    FragColor = Color;                                                              \n\
}";

static void RenderSceneCB()
{
    glClear(GL_COLOR_BUFFER_BIT);// Очистка буфера цвета
    static float Scale = 0.0f;// Статическая переменная, хранящая текущий масштаб
    Scale += 0.1f;// Увеличение масштаба на 0.1
    Pipeline p;// Создание экземпляра объекта Pipeline
    p.Rotate(0.0f, Scale, 0.0f);// Вращение объекта вокруг оси Y на Scale градусов
    p.WorldPos(0.0f, 0.0f, 3.0f);// Перемещение объекта вдоль оси Z на 3 единицы
    Vector3f CameraPos(0.0f, 0.0f, -3.0f);// Установка позиции камеры в точку (0, 0, -3)
    Vector3f CameraTarget(0.0f, 0.0f, 2.0f);// Установка цели камеры в точку (0, 0, 2)
    Vector3f CameraUp(0.0f, 1.0f, 0.0f);// Установка направления "вверх" камеры вдоль оси Y
    p.SetCamera(CameraPos, CameraTarget, CameraUp);// Установка камеры в объект Pipeline
    p.SetPerspectiveProj(30.0f, WINDOW_WIDTH, WINDOW_HEIGHT, 1.0f, 100.0f);// Установка параметров перспективной проекции
    glUniformMatrix4fv(gWVPLocation, 1, GL_TRUE, (const GLfloat*)p.GetTrans());// Установка матрицы преобразования в шейдере
    glEnableVertexAttribArray(0);// Включение массива вершин атрибута 0
    glBindBuffer(GL_ARRAY_BUFFER, VBO);// Привязка буфера вершин
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);// Установка указателя на массив вершин
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);// Привязка буфера индексов
    glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);// Отрисовка элементов
    glDisableVertexAttribArray(0);// Отключение массива вершин атрибута 0
    glutSwapBuffers();// Обмен буферов
}


static void InitializeGlutCallbacks()
{
    glutDisplayFunc(RenderSceneCB); // Устанавливает функцию обратного вызова для отображения сцены
    glutIdleFunc(RenderSceneCB); // Устанавливает функцию обратного вызова, которая вызывается, когда приложение находится в режиме простоя и не выполняет других задач.
}

static void CreateVertexBuffer()
{
    Vector3f Vertices[4];// Создание массива вершин
    Vertices[0] = Vector3f(-1.0f, -1.0f, 0.5773f); // Инициализация первой вершины в массиве
    Vertices[1] = Vector3f(0.0f, -1.0f, -1.15475);// Инициализация второй вершины в массиве
    Vertices[2] = Vector3f(1.0f, -1.0f, 0.5773f); // Инициализация третьей вершины в массиве
    Vertices[3] = Vector3f(0.0f, 1.0f, 0.0f);// Инициализация четвертой вершины в массиве

    glGenBuffers(1, &VBO); // Создание буфера вершин
    glBindBuffer(GL_ARRAY_BUFFER, VBO);// Привязка буфера вершин к массиву вершин
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW); // Загрузка массива вершин в буфер вершин
}

static void CreateIndexBuffer()
{
    // Объявляем массив беззнаковых целых чисел и инициализируем его
    unsigned int Indices[] = { 0, 3, 1,
                               1, 3, 2,
                               2, 3, 0,
                               0, 2, 1 };


    glGenBuffers(1, &IBO);// Генерируем буфер и сохраняем его идентификатор в переменную IBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO); // Привязываем буфер типа GL_ELEMENT_ARRAY_BUFFER к контексту OpenGL
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW); // Заполняем буфер данными из массива Indices
}

static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
    GLuint ShaderObj = glCreateShader(ShaderType);// создаем объект шейдера, передавая тип шейдера (Vertex или Fragment) в качестве аргумента

    // проверяем успешно ли создан объект шейдера, и если нет, выводим сообщение об ошибке и выходим из программы
    if (ShaderObj == 0) {
        fprintf(stderr, "Error creating shader type %d\n", ShaderType);
        exit(0);
    }

    // задаем исходный текст для компиляции шейдера
    const GLchar* p[1];
    p[0] = pShaderText;
    GLint Lengths[1];
    Lengths[0] = strlen(pShaderText);
    glShaderSource(ShaderObj, 1, p, Lengths);

    glCompileShader(ShaderObj);// компилируем шейдерный объект

    // получаем статус компиляции шейдера
    GLint success;
    glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);

    // если компиляция не прошла успешно, выводим сообщение об ошибке и выходим из программы
    if (!success) {
        GLchar InfoLog[1024];
        glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
        fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
        exit(1);
    }

    glAttachShader(ShaderProgram, ShaderObj);// прикрепляем шейдерный объект к программе шейдеров
}

static void CompileShaders()
{
    GLuint ShaderProgram = glCreateProgram();// Создание объекта шейдерной программы и получение его идентификатора

    // Проверка, что объект шейдерной программы был успешно создан
    if (ShaderProgram == 0) {
        fprintf(stderr, "Error creating shader program\n");
        exit(1);
    }

    // Добавление вершинного и фрагментного шейдеров к шейдерной программе
    AddShader(ShaderProgram, pVS, GL_VERTEX_SHADER);
    AddShader(ShaderProgram, pFS, GL_FRAGMENT_SHADER);

    // Инициализация переменных Success и ErrorLog
    GLint Success = 0;
    GLchar ErrorLog[1024] = { 0 };

    // Привязка всех шейдеров к шейдерной программе и проверка успешности связывания
    glLinkProgram(ShaderProgram);
    glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &Success);
    if (Success == 0) {
        glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
        exit(1);
    }

    // Проверка валидности шейдерной программы
    glValidateProgram(ShaderProgram);
    glGetProgramiv(ShaderProgram, GL_VALIDATE_STATUS, &Success);
    if (!Success) {
        glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
        exit(1);
    }

    glUseProgram(ShaderProgram);// Использование шейдерной программы
    gWVPLocation = glGetUniformLocation(ShaderProgram, "gWVP"); // Получение местоположения переменной uniform в шейдерной программе
    assert(gWVPLocation != 0xFFFFFFFF);// Проверка, что переменная была найдена
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);// Инициализация GLUT с передачей аргументов argc и argv в функцию glutInit 
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);// Установка параметров отображения: двойная буферизация, цветовая модель RGBA
    glutInitWindowSize(1024, 768); // Установка размеров окна
    glutInitWindowPosition(100, 100);// Установка позиции окна
    glutCreateWindow("IKG_LAB_2"); // Создание окна с названием "IKG_LAB_2"  
    InitializeGlutCallbacks();// Инициализация обратных вызовов GLUT

    // Инициализация GLEW и проверка на ошибки
    GLenum res = glewInit();
    if (res != GLEW_OK) {
        // Вывод сообщения об ошибке с использованием функции fprintf и возвращение значения 1
        fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
        return 1;
    }

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);// Установка цвета фона 
    CreateVertexBuffer();// Создание буфера вершин
    CreateIndexBuffer();// Создание буфера индекса
    CompileShaders(); // Компиляция шейдеров
    glutMainLoop();// Запуск бесконечного цикла обработки сообщений GLUT

    return 0;
}
