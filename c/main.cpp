#include <Box2D/Box2D.h>
#include <cstdio>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <stdarg.h>
#include <iostream>
#include <string>  
#include "engineGlobals.h"

#define bool int
#define true 1
#define false 0
#define GL_LOG_FILE "gl.log"



bool restart_gl_log ();
bool gl_log (const char* message, ...) ;
bool gl_log_err (const char* message, ...);
void log_gl_params () ;
void _update_fps_counter (GLFWwindow* window);
void init_settings();



/* we will tell GLFW to run this function whenever it finds an error */
void glfw_error_callback (int error, const char* description) {
    gl_log_err ("GLFW ERROR: code %i msg: %s\n", error, description);
}
/* we will tell GLFW to run this function whenever the window is resized */
void glfw_window_size_callback (GLFWwindow* window, int width, int height) {
    g_gl_width = width;
    g_gl_height = height;
    printf ("width %i height %i\n", width, height);
    /* update any perspective matrices used here */
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_A && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}



int main (int argc, char* argv[]) {
	
	
	int g_gl_width = atoi(argv[1]);
	int g_gl_height = atoi(argv[2]);
	int g_gl_pos_x = atoi(argv[3]);
	int g_gl_pos_y = atoi(argv[4]);
	
    GLFWwindow* window;

    GLfloat points[] = {
            1.0f,	0.5f,	0.0f,
            0.5f, -0.5f,	0.0f,
            -0.5f, -0.5f,	0.0f
    };
    GLuint vbo;
    GLuint vao;



    GLuint shader_programme, vs, fs;



    glfwSetErrorCallback (glfw_error_callback);
    if (!glfwInit ()) {
        fprintf (stderr, "ERROR: could not start GLFW3\n");
        return 1;
    }

    /* we can run a full-screen window here */

    /*GLFWmonitor* mon = glfwGetPrimaryMonitor ();
    const GLFWvidmode* vmode = glfwGetVideoMode (mon);
    GLFWwindow* window = glfwCreateWindow (
        vmode->width, vmode->height, "Extended GL Init", mon, NULL
    );*/
	glfwWindowHint(GLFW_DECORATED, GL_FALSE);
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    window = glfwCreateWindow (
            g_gl_width, g_gl_height, "Extended Init.", NULL, NULL
    );
    if (!window) {
        fprintf (stderr, "ERROR: could not open window with GLFW3\n");
        glfwTerminate();
        return 1;
    }
	glfwSetWindowPos(window, g_gl_pos_x, g_gl_pos_y);
    glfwSetWindowSizeCallback (window, glfw_window_size_callback);
    glfwMakeContextCurrent (window);
	glfwSetKeyCallback(window, key_callback);

    glfwWindowHint (GLFW_SAMPLES, 4);
    init_settings();


    // get version info


    glGenBuffers (1, &vbo);
    glBindBuffer (GL_ARRAY_BUFFER, vbo);
    glBufferData (GL_ARRAY_BUFFER, 9 * sizeof (GLfloat), points, GL_STATIC_DRAW);

    glGenVertexArrays (1, &vao);
    glBindVertexArray (vao);
    glEnableVertexAttribArray (0);
    glBindBuffer (GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    vs = glCreateShader (GL_VERTEX_SHADER);
    glShaderSource (vs, 1, &vertex_shader, NULL);
    glCompileShader (vs);
    fs = glCreateShader (GL_FRAGMENT_SHADER);
    glShaderSource (fs, 1, &fragment_shader, NULL);
    glCompileShader (fs);
    shader_programme = glCreateProgram ();
    glAttachShader (shader_programme, fs);
    glAttachShader (shader_programme, vs);
    glLinkProgram (shader_programme);



    b2Vec2 gravity(0.0f, -10.0f);

    printf("%4.2f" , gravity.y);

    ImVec4 clear_color = ImColor(114, 144, 154);
    printf("%4.2f" , clear_color.x);






    while (!glfwWindowShouldClose (window)) {
        _update_fps_counter (window);
        // wipe the drawing surface clear
        glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport (0, 0, g_gl_width, g_gl_height);

        glUseProgram (shader_programme);
        glBindVertexArray (vao);
        // draw points 0-3 from the currently bound VAO with current in-use shader
        glDrawArrays (GL_TRIANGLES, 0, 3);
        // update other events like input handling
        glfwPollEvents ();
        if (GLFW_PRESS == glfwGetKey (window, GLFW_KEY_ESCAPE)) {
            glfwSetWindowShouldClose (window, 1);
        }
        // put the stuff we've been drawing onto the display
        glfwSwapBuffers (window);



        //printf("%4.2f %4.2f %4.2f\n", position.x, position.y, angle);


    }

    // close GL context and any other GLFW resources
    glfwTerminate();
    return 0;
}











/* start a new log file. put the time and date at the top */
bool restart_gl_log () {
    time_t now;
    char* date;
    FILE* file = fopen (GL_LOG_FILE, "w");

    if (!file) {
        fprintf (
                stderr,
                "ERROR: could not open GL_LOG_FILE log file %s for writing\n",
                GL_LOG_FILE
        );
        return false;
    }
    now = time (NULL);
    date = ctime (&now);
    fprintf (file, "GL_LOG_FILE log. local time %s", date);
    fprintf (file, "build version: %s %s\n\n", __DATE__, __TIME__);
    fclose (file);
    return true;
}

/* add a message to the log file. arguments work the same way as printf() */
bool gl_log (const char* message, ...) {
    va_list argptr;
    FILE* file = fopen (GL_LOG_FILE, "a");
    if (!file) {
        fprintf (
                stderr,
                "ERROR: could not open GL_LOG_FILE %s file for appending\n",
                GL_LOG_FILE
        );
        return false;
    }
    va_start (argptr, message);
    vfprintf (file, message, argptr);
    va_end (argptr);
    fclose (file);
    return true;
}

/* same as gl_log except also prints to stderr */
bool gl_log_err (const char* message, ...) {
    va_list argptr;
    FILE* file = fopen (GL_LOG_FILE, "a");
    if (!file) {
        fprintf (
                stderr,
                "ERROR: could not open GL_LOG_FILE %s file for appending\n",
                GL_LOG_FILE
        );
        return false;
    }
    va_start (argptr, message);
    vfprintf (file, message, argptr);
    va_end (argptr);
    va_start (argptr, message);
    vfprintf (stderr, message, argptr);
    va_end (argptr);
    fclose (file);
    return true;
}

/* we can use a function like this to print some GL capabilities of our adapter
to the log file. handy if we want to debug problems on other people's computers
*/
void log_gl_params () {
    int i;
    int v[2];
    unsigned char s = 0;
    GLenum params[] = {
            GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS,
            GL_MAX_CUBE_MAP_TEXTURE_SIZE,
            GL_MAX_DRAW_BUFFERS,
            GL_MAX_FRAGMENT_UNIFORM_COMPONENTS,
            GL_MAX_TEXTURE_IMAGE_UNITS,
            GL_MAX_TEXTURE_SIZE,
            GL_MAX_VARYING_FLOATS,
            GL_MAX_VERTEX_ATTRIBS,
            GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS,
            GL_MAX_VERTEX_UNIFORM_COMPONENTS,
            GL_MAX_VIEWPORT_DIMS,
            GL_STEREO,
    };
    const char* names[] = {
            "GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS",
            "GL_MAX_CUBE_MAP_TEXTURE_SIZE",
            "GL_MAX_DRAW_BUFFERS",
            "GL_MAX_FRAGMENT_UNIFORM_COMPONENTS",
            "GL_MAX_TEXTURE_IMAGE_UNITS",
            "GL_MAX_TEXTURE_SIZE",
            "GL_MAX_VARYING_FLOATS",
            "GL_MAX_VERTEX_ATTRIBS",
            "GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS",
            "GL_MAX_VERTEX_UNIFORM_COMPONENTS",
            "GL_MAX_VIEWPORT_DIMS",
            "GL_STEREO",
    };
    gl_log ("GL Context Params:\n");
    // integers - only works if the order is 0-10 integer return types
    for (i = 0; i < 10; i++) {
        int v = 0;
        glGetIntegerv (params[i], &v);
        gl_log ("%s %i\n", names[i], v);
    }
    // others
    v[0] = v[1] = 0;
    glGetIntegerv (params[10], v);
    gl_log ("%s %i %i\n", names[10], v[0], v[1]);
    glGetBooleanv (params[11], &s);
    gl_log ("%s %i\n", names[11], (unsigned int)s);
    gl_log ("-----------------------------\n");
}

/* we will use this function to update the window title with a frame rate */
void _update_fps_counter (GLFWwindow* window) {
    double current_seconds;
    double elapsed_seconds;
    char tmp[128];

    current_seconds = glfwGetTime ();
    elapsed_seconds = current_seconds - previous_seconds;
    if (elapsed_seconds > 0.25) {
        previous_seconds = current_seconds;

        double fps = (double)frame_count / elapsed_seconds;
        sprintf (tmp, "opengl @ fps: %.2f", fps);
        glfwSetWindowTitle (window, tmp);
        frame_count = 0;
    }
    frame_count++;
}

void init_settings(){

    restart_gl_log ();
    // start GL context and O/S window using the GLFW helper library
    gl_log ("starting GLFW\n%s\n", glfwGetVersionString ());
    glewExperimental = GL_TRUE;
    glewInit ();
    renderer = glGetString (GL_RENDERER);
    version = glGetString (GL_VERSION);
    printf ("Renderer: %s\n", renderer);
    printf ("OpenGL version supported %s\n", version);
    gl_log ("renderer: %s\nversion: %s\n", renderer, version);
    log_gl_params ();

    glEnable (GL_DEPTH_TEST); // enable depth-testing
    glDepthFunc (GL_LESS); // depth-testing interprets a smaller value as "closer"
}
