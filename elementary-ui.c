#include <Evas.h>
#include <Ecore.h>
#include <Evas_Engine_Buffer.h>
#include <Elementary.h>

#include "glhck/glhck.h"
#include <stdio.h>
#include "GLFW/glfw3.h"
#include "glfwhck.h"

int const WINDOW_WIDTH = 800;
int const WINDOW_HEIGHT = 480;

int const UI_WIDTH = 600;
int const UI_HEIGHT = 400;

GLFWwindow* window = NULL;

int init(int argc, char** argv);
int run();
int deinit();

void errorCallback(int code, char const* message);

void elmButtonClicked(void *data, Evas_Object *o, void *event_info);

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

  window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "glhck-evas elementary-ui", NULL, NULL);

  if(!window)
  {
    return EXIT_FAILURE;
  }

  glfwMakeContextCurrent(window);
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
  elm_init(argc, argv);

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

  Evas_Object* bg = evas_object_rectangle_add(canvas);
  evas_object_color_set(bg, 240, 0, 240, 255);
  evas_object_move(bg, 0, 0);
  evas_object_resize(bg, UI_WIDTH, UI_HEIGHT);
  evas_object_show(bg);

  Evas_Object* btn = elm_button_add(bg);
  elm_object_text_set(btn, "Muffin!");
  evas_object_move(btn, UI_WIDTH/2-50, UI_HEIGHT-50);
  evas_object_resize(btn, 100, 44);
  evas_object_show(btn);

  Evas_Object* cursor = evas_object_rectangle_add(canvas);
  evas_object_color_set(cursor, 255, 255, 255, 255);
  evas_object_resize(cursor, 2, 2);
  evas_object_pass_events_set(cursor, 1);
  evas_object_show(cursor);
  evas_object_raise(cursor);

  evas_object_smart_callback_add(btn, "clicked", elmButtonClicked, NULL);


  glfwSetWindowUserPointer(window, canvas);
  glhckTextureFill(texture, 0, 0, 0, 0, UI_WIDTH, UI_HEIGHT, 0,
                   GLHCK_RGBA, GLHCK_UNSIGNED_BYTE, UI_WIDTH * UI_HEIGHT, einfo->info.dest_buffer);

  glfwhckEventQueue* queue = glfwhckEventQueueNew(window, GLFWHCK_EVENTS_ALL);
  char running = 1;

  while(running)
  {
    glfwPollEvents();

    while(!glfwhckEventQueueEmpty(queue))
    {
      glfwhckEvent const* event = glfwhckEventQueuePop(queue);
      switch(event->type)
      {
        case GLFWHCK_EVENT_WINDOW_CLOSE:
          running = 0;
          break;
        case GLFWHCK_EVENT_KEYBOARD_KEY:
          if(event->keyboardKey.key == GLFW_KEY_ESCAPE)
          {
            running = 0;
          }
          break;
        case GLFWHCK_EVENT_MOUSE_POSITION:
          evas_event_feed_mouse_move(canvas, event->mousePosition.x, event->mousePosition.y, glfwGetTime() * 1000000, NULL);
          evas_object_move(cursor, event->mousePosition.x, event->mousePosition.y);
          break;
        case GLFWHCK_EVENT_MOUSE_BUTTON:
          if(event->mouseButton.action == GLFW_PRESS)
          {
            evas_event_feed_mouse_down(canvas, event->mouseButton.button + 1, EVAS_BUTTON_NONE, glfwGetTime() * 1000000, NULL);
          }
          else if(event->mouseButton.action == GLFW_RELEASE)
          {
            evas_event_feed_mouse_up(canvas, event->mouseButton.button + 1, EVAS_BUTTON_NONE, glfwGetTime() * 1000000, NULL);
          }
          break;
        case GLFWHCK_EVENT_MOUSE_ENTER:
          if(event->mouseEnter.entered == GL_TRUE)
          {
            evas_event_feed_mouse_in(canvas, glfwGetTime() * 1000000, NULL);
          }
          else if(event->mouseEnter.entered == GL_FALSE)
          {
            evas_event_feed_mouse_out(canvas, glfwGetTime() * 1000000, NULL);
          }
          break;
        default:
          printf("Unhandled event: %d\n", event->type);
          break;
      }

    }

    evas_async_events_process();
    ecore_main_loop_iterate();

    Eina_List* updates = evas_render_updates(canvas);
    Eina_List* n;
    Eina_Rectangle* update;
    EINA_LIST_FOREACH(updates, n, update)
      glhckTextureFillFrom(texture, 0, update->x, update->y, 0,  update->x, update->y, 0, update->w, update->h, 0,
                       GLHCK_RGBA, GLHCK_UNSIGNED_BYTE, UI_WIDTH * UI_HEIGHT, einfo->info.dest_buffer);

    evas_render_updates_free(updates);

    glhckRenderFlip(0);
    glhckRenderCullFace(GLHCK_FRONT);
    glhckObjectRender(object);
    glfwSwapBuffers(window);
    glhckRenderClear(GLHCK_DEPTH_BUFFER_BIT | GLHCK_COLOR_BUFFER_BIT);
  }

  glfwhckEventQueueFree(queue);
  free(einfo->info.dest_buffer);
  evas_free(canvas);
  glhckObjectFree(object);

  return EXIT_SUCCESS;
}

int deinit()
{
  elm_shutdown();
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

void elmButtonClicked(void *data, Evas_Object *o, void *event_info)
{
  evas_object_color_set(o, 255, 255, 255, 255);
  printf("CLICKED\n");
}
