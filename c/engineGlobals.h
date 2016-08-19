const char* vertex_shader =
        "#version 410\n"
                "in vec3 vp;"
                "void main () {"
                "	gl_Position = vec4 (vp, 1.0);"
                "}";

const char* fragment_shader =
        "#version 410\n"
                "out vec4 frag_colour;"
                "void main () {"
                "	frag_colour = vec4 (0.5, 0.0, 0.5, 1.0);"
                "}";

int g_gl_width = 640;
int g_gl_height = 480;

double previous_seconds;
int frame_count;

float GRAVITY_Y = -10.0f;


const GLubyte* renderer;
const GLubyte* version;