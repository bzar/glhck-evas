#include <Evas.h>
#include <Ecore.h>
#include <Evas_Engine_Buffer.h>

#include "glhck/glhck.h"
#include <stdio.h>
#include "GLFW/glfw3.h"

int const WINDOW_WIDTH = 800;
int const WINDOW_HEIGHT = 480;

int const UI_WIDTH = 600;
int const UI_HEIGHT = 400;

char running = 1;
GLFWwindow* window = NULL;

int init(int argc, char** argv);
int run();
int deinit();

void errorCallback(int code, char const* message);
void windowCloseCallback(GLFWwindow* window);
void windowSizeCallback(GLFWwindow *handle, int width, int height);
void cursorPosCallback(GLFWwindow* window, double x, double y);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void cursorEnterCallback(GLFWwindow* window, int value);
void evasMouseDownCallback(void *data, Evas *evas, Evas_Object *o, void *einfo);
void evasMouseUpCallback(void *data, Evas *evas, Evas_Object *o, void *einfo);

int init(int argc, char** argv)
{
  // GLFW

  if (!glfwInit())
  {
    printf("GLFW initialization error\n");
    return EXIT_FAILURE;
  }
  glfwSetErrorCallback(errorCallback);

  glhckCompileFeatures features;
  glhckGetCompileFeatures(&features);
  glfwDefaultWindowHints();
  glfwWindowHint(GLFW_SAMPLES, 4);
  if(features.render.glesv1 || features.render.glesv2) {
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_DEPTH_BITS, 16);
  }
  if(features.render.glesv2) {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  }
  if(features.render.opengl) {
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
  }

  window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "glhck-evas mouse-events", NULL, NULL);

  if(!window)
  {
    return EXIT_FAILURE;
  }

  glfwMakeContextCurrent(window);

  glfwSetWindowCloseCallback(window, windowCloseCallback);
  glfwSetWindowSizeCallback(window, windowSizeCallback);
  glfwSetCursorPosCallback(window, cursorPosCallback);
  glfwSetMouseButtonCallback(window, mouseButtonCallback);
  glfwSetCursorEnterCallback(window, cursorEnterCallback);


  glfwSwapInterval(1);

  // GLHCK

  if(!glhckContextCreate(argc, argv))
  {
    printf("Failed to create a GLhck context\n");
    return EXIT_FAILURE;
  }

  glhckLogColor(0);
  if(!glhckDisplayCreate(WINDOW_WIDTH, WINDOW_HEIGHT, GLHCK_RENDER_AUTO))
  {
    printf("Failed to create a GLhck display\n");
    return EXIT_FAILURE;
  }

  glhckRenderClearColorb(64, 64, 64, 255);

  // ECORE / EVAS
  ecore_init();
  evas_init();

  return EXIT_SUCCESS;
}

int run()
{
  glhckObject* object = glhckPlaneNew(UI_WIDTH, UI_HEIGHT);
  glhckObjectPositionf(object, WINDOW_WIDTH/2.0f, WINDOW_HEIGHT/2.0f, 0);

  glhckTexture* texture = glhckTextureNew();
  glhckTextureCreate(texture, GLHCK_TEXTURE_2D, 0, UI_WIDTH, UI_HEIGHT, 0, 0, GLHCK_RGBA, GLHCK_UNSIGNED_BYTE, UI_WIDTH * UI_HEIGHT, NULL);
  glhckTextureParameter(texture, glhckTextureDefaultParameters());
  glhckMaterial* material = glhckMaterialNew(texture);
  glhckTextureFree(texture);
  glhckObjectMaterial(object, material);
  glhckMaterialFree(material);

  Evas* canvas = evas_new();
  int method = evas_render_method_lookup("buffer");
  evas_output_method_set(canvas, method);
  evas_output_size_set(canvas, UI_WIDTH, UI_HEIGHT);
  evas_output_viewport_set(canvas, 0, 0, UI_WIDTH, UI_HEIGHT);
  Evas_Engine_Info_Buffer* einfo = (Evas_Engine_Info_Buffer *) evas_engine_info_get(canvas);
  einfo->info.depth_type = EVAS_ENGINE_BUFFER_DEPTH_ARGB32;
  einfo->info.dest_buffer = malloc(UI_WIDTH * UI_HEIGHT * sizeof(int));
  einfo->info.dest_buffer_row_bytes = UI_WIDTH * sizeof(int);
  einfo->info.use_color_key = 0;
  einfo->info.alpha_threshold = 0;
  einfo->info.func.new_update_region = NULL;
  einfo->info.func.free_update_region = NULL;
  evas_engine_info_set(canvas, (Evas_Engine_Info *)einfo);

  Evas_Object* bg;
  bg = evas_object_rectangle_add(canvas);
  evas_object_color_set(bg, 240, 0, 240, 255);
  evas_object_move(bg, 0, 0);
  evas_object_resize(bg, UI_WIDTH, UI_HEIGHT);
  evas_object_show(bg);

  Evas_Object* eobj;
  eobj = evas_object_rectangle_add(canvas);
  evas_object_color_set(eobj, 128, 255, 128, 255);
  evas_object_move(eobj, 100, 100);
  evas_object_resize(eobj, 100, 100);
  evas_object_show(eobj);
  evas_object_focus_set(eobj, EINA_TRUE);
  evas_object_raise(eobj);

  evas_object_event_callback_add(bg, EVAS_CALLBACK_MOUSE_DOWN, evasMouseDownCallback, NULL);
  evas_object_event_callback_add(bg, EVAS_CALLBACK_MOUSE_UP, evasMouseUpCallback, NULL);

  glfwSetWindowUserPointer(window, canvas);
  glhckTextureFill(texture, 0, 0, 0, 0, UI_WIDTH, UI_HEIGHT, 0,
                   GLHCK_RGBA, GLHCK_UNSIGNED_BYTE, UI_WIDTH * UI_HEIGHT, einfo->info.dest_buffer);
  while(running)
  {
    glfwPollEvents();

    Eina_List* updates = evas_render_updates(canvas);
    Eina_List* n;
    Eina_Rectangle* update;
    EINA_LIST_FOREACH(updates, n, update)
      glhckTextureFillFrom(texture, 0, update->x, update->y, 0,  update->x, update->y, 0, update->w, update->h, 0,
                       GLHCK_RGBA, GLHCK_UNSIGNED_BYTE, UI_WIDTH * UI_HEIGHT, einfo->info.dest_buffer);

    evas_render_updates_free(updates);

    glhckObjectRender(object);
    glfwSwapBuffers(window);
    glhckRenderClear(GLHCK_DEPTH_BUFFER_BIT | GLHCK_COLOR_BUFFER_BIT);
  }

  free(einfo->info.dest_buffer);
  evas_free(canvas);
  glhckObjectFree(object);

  return EXIT_SUCCESS;
}

int deinit()
{
  glhckContextTerminate();
  glfwTerminate();
  return EXIT_SUCCESS;
}

int main(int argc, char** argv)
{
  int initErr = init(argc, argv);

  if(initErr)
    return initErr;

  int runErr = run();

  if(runErr)
    return runErr;

  int deinitErr = deinit();

  if(deinitErr)
    return deinitErr;

  return EXIT_SUCCESS;
}

void errorCallback(int code, char const* message)
{
  printf("GLFW ERROR: %s\n", message);
}

void windowCloseCallback(GLFWwindow* window)
{
  running = 0;
}

void windowSizeCallback(GLFWwindow *window, int width, int height)
{
  glhckDisplayResize(width, height);
}


void cursorPosCallback(GLFWwindow* window, double x, double y)
{
  Evas* canvas = (Evas*) glfwGetWindowUserPointer(window);
  evas_event_feed_mouse_move(canvas, x, y, glfwGetTime() * 1000000, NULL);
}

void cursorEnterCallback(GLFWwindow* window, int value)
{
  Evas* canvas = (Evas*) glfwGetWindowUserPointer(window);
  if(value == GL_TRUE)
  {
    evas_event_feed_mouse_in(canvas, glfwGetTime() * 1000000, NULL);
  }
  else if(value == GL_FALSE)
  {
    evas_event_feed_mouse_out(canvas, glfwGetTime() * 1000000, NULL);
  }

}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
  Evas* canvas = (Evas*) glfwGetWindowUserPointer(window);

  // TODO: handle double/triple clicks
  if(action == GLFW_PRESS)
  {
    evas_event_feed_mouse_down(canvas, button + 1, EVAS_BUTTON_NONE, glfwGetTime() * 1000000, NULL);
  }
  else if(action == GLFW_RELEASE)
  {
    evas_event_feed_mouse_up(canvas, button + 1, EVAS_BUTTON_NONE, glfwGetTime() * 1000000, NULL);
  }
}

void evasMouseDownCallback(void *data, Evas *evas, Evas_Object *o, void *einfo)
{
  evas_object_color_set(o, 0, 255, 0, 255);
}

void evasMouseUpCallback(void *data, Evas *evas, Evas_Object *o, void *einfo)
{
  evas_object_color_set(o, 255, 0, 255, 255);
}
