#include <Evas.h>
#include <Ecore.h>
#include <Evas_Engine_Buffer.h>
#include <Elementary.h>

#include "glhck/glhck.h"
#include <stdio.h>
#include "GLFW/glfw3.h"
#include "glfwhck.h"

int WINDOW_WIDTH = 800;
int WINDOW_HEIGHT = 480;

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
  elm_init(argc, argv);

  return EXIT_SUCCESS;
}

int run()
{
  glhckSetGlobalPrecision(GLHCK_IDX_USHRT, GLHCK_VTX_V3F);
  glhckObject* object = glhckPlaneNew(UI_WIDTH, UI_HEIGHT);
  glhckObjectPositionf(object, WINDOW_WIDTH/2.0f, WINDOW_HEIGHT/2.0f, 0);

  glhckTexture* texture = glhckTextureNew();
  glhckTextureCreate(texture, GLHCK_TEXTURE_2D, 0, UI_WIDTH, UI_HEIGHT, 0, 0,
                     GLHCK_RGBA, GLHCK_UNSIGNED_BYTE, UI_WIDTH * UI_HEIGHT, NULL);
  glhckTextureParameter(texture, glhckTextureDefaultLinearParameters());
  glhckMaterial* material = glhckMaterialNew(texture);
  glhckMaterialBlendFunc(material, GLHCK_SRC_ALPHA, GLHCK_ONE_MINUS_SRC_ALPHA);
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
  evas_object_color_set(bg, 240, 0, 240, 0);
  evas_object_move(bg, 0, 0);
  evas_object_resize(bg, UI_WIDTH, UI_HEIGHT);
  evas_object_show(bg);

  Evas_Object* btn = elm_button_add(bg);
  elm_object_text_set(btn, "Muffin!");
  evas_object_move(btn, UI_WIDTH/2-50, UI_HEIGHT-50);
  evas_object_resize(btn, 100, 44);
  evas_object_show(btn);

  Evas_Object* label = elm_label_add(bg);
  elm_object_text_set(label, "O HAI!");
  evas_object_show(label);

  Evas_Object* bubble = elm_bubble_add(bg);
  evas_object_move(bubble, 0, -18);
  evas_object_resize(bubble, 60, 50);
  elm_object_content_set(bubble, label);
  evas_object_show(bubble);

  Evas_Object* as = elm_actionslider_add(bg);
  evas_object_move(as, UI_WIDTH/2-75, 0);
  evas_object_resize(as, 150, 50);
  elm_actionslider_indicator_pos_set(as, ELM_ACTIONSLIDER_CENTER);
  elm_actionslider_magnet_pos_set(as, ELM_ACTIONSLIDER_CENTER);
  elm_object_part_text_set(as, "left", "Die");
  elm_object_part_text_set(as, "right", "Live");
  elm_actionslider_enabled_pos_set(as, ELM_ACTIONSLIDER_LEFT | ELM_ACTIONSLIDER_RIGHT);
  evas_object_show(as);

  Evas_Object* cal = elm_calendar_add(bg);
  evas_object_resize(cal, 200, 200);
  evas_object_size_hint_weight_set(cal, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
  evas_object_show(cal);

  Evas_Object* f1 = elm_frame_add(bg);
  evas_object_move(f1, UI_WIDTH-200, UI_HEIGHT-200);
  evas_object_resize(f1, 200, 200);
  elm_object_content_set(f1, cal);
  elm_object_text_set(f1, "Calendar");
  evas_object_show(f1);

  Evas_Object* ck = elm_clock_add(bg);
  evas_object_resize(ck, 100, 80);
  elm_clock_show_seconds_set(ck, EINA_TRUE);
  evas_object_show(ck);

  Evas_Object* f2 = elm_frame_add(bg);
  evas_object_move(f2, 0, UI_HEIGHT-80);
  evas_object_resize(f2, 100, 80);
  elm_object_content_set(f2, ck);
  elm_object_text_set(f2, "Clock");
  evas_object_show(f2);

  Evas_Object* ewidth = elm_entry_add(bg);
  evas_object_resize(ewidth, 150, 44);
  elm_entry_single_line_set(ewidth, EINA_TRUE);
  elm_object_text_set(ewidth, "Type something...");
  evas_object_size_hint_weight_set(ewidth, EVAS_HINT_EXPAND, 0.0);
  evas_object_size_hint_align_set(ewidth, EVAS_HINT_FILL, EVAS_HINT_FILL);
  evas_object_show(ewidth);

  Evas_Object* f3 = elm_frame_add(bg);
  evas_object_move(f3, UI_WIDTH-150, 0);
  evas_object_resize(f3, 150, 44);
  elm_object_content_set(f3, ewidth);
  elm_object_text_set(f3, "Entry box");
  evas_object_show(f3);

  Evas_Object* pb = elm_progressbar_add(bg);
  evas_object_move(pb, 70, 10);
  evas_object_resize(pb, 20, 20);
  evas_object_size_hint_align_set(pb, EVAS_HINT_FILL, 0.5);
  evas_object_size_hint_weight_set(pb, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
  elm_progressbar_pulse_set(pb, EINA_TRUE);
  elm_progressbar_pulse(pb, EINA_TRUE);
  evas_object_show(pb);

  Evas_Object* pb2 = elm_progressbar_add(bg);
  evas_object_move(pb2, 100, 10);
  evas_object_resize(pb2, 100, 20);
  evas_object_size_hint_align_set(pb2, EVAS_HINT_FILL, 0.5);
  evas_object_size_hint_weight_set(pb2, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
  elm_progressbar_value_set(pb2, 0.5f);
  evas_object_show(pb2);

  Evas_Object* vf = elm_video_add(bg);
  /*elm_video_file_set(vf, "http://mirror.bigbuckbunny.de/peach/bigbuckbunny_movies/big_buck_bunny_480p_stereo.ogg");
  elm_video_play(vf);*/

  Evas_Object* vd = elm_player_add(bg);
  evas_object_resize(vd, 260, 260);
  evas_object_move(vd, UI_WIDTH/2-180, UI_HEIGHT/2-130);
  elm_object_content_set(vd, vf);
  evas_object_show(vd);

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

  glhckText *text = glhckTextNew(512, 512);
  unsigned int font = glhckTextFontNew(text, "/usr/share/fonts/TTF/DejaVuSans.ttf");
  glhckObject *rttText = glhckTextPlane(text, font, 42, "GLHCK IS AWESOME!", NULL);

  glhckObject *cube = glhckCubeNew(50.0f);
  glhckObjectMaterial(cube, material);

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
        case GLFWHCK_EVENT_WINDOW_RESIZE:
          WINDOW_WIDTH = event->windowResize.width;
          WINDOW_HEIGHT = event->windowResize.height;
          glhckDisplayResize(WINDOW_WIDTH, WINDOW_HEIGHT);
        case GLFWHCK_EVENT_KEYBOARD_KEY:
          if(event->keyboardKey.key == GLFW_KEY_ESCAPE)
          {
            running = 0;
          }
          break;
        case GLFWHCK_EVENT_MOUSE_POSITION:
        {
          kmRay3 mouseRay = {
            {event->mousePosition.x, event->mousePosition.y, -1000},
            {0, 0, 1}
          };

          kmVec2 coords;
          if(glhckObjectPickTextureCoordinatesWithRay(object, &mouseRay, &coords))
          {
            evas_event_feed_mouse_move(canvas, coords.x * UI_WIDTH, coords.y * UI_HEIGHT, glfwGetTime() * 1000000, NULL);
            evas_object_move(cursor, coords.x * UI_WIDTH, coords.y * UI_HEIGHT);
          }
          break;
        }
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
    glhckObjectRender(cube);
    glhckObjectRotatef(cube, 0.0f, 1.0f, 0.0f);
    glhckObjectPositionf(cube, 100, 200, -100.0f);

    glhckRenderFlip(1);
    glhckRenderCullFace(GLHCK_BACK);
    glhckObjectPositionf(rttText, sin(glfwGetTime()) * 200.0f + 800/2, 480/2, -10.0f);
    glhckObjectRender(rttText);

    if (glfwGetKey(window, GLFW_KEY_SPACE)) {
      glhckObjectRotatef(object, 0, 0, 1.0f);
    }

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
