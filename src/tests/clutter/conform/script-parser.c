#include <stdlib.h>
#include <string.h>

#define CLUTTER_DISABLE_DEPRECATION_WARNINGS
#include <clutter/clutter.h>

#include "tests/clutter-test-utils.h"

#define TEST_TYPE_GROUP                 (test_group_get_type ())
#define TEST_TYPE_GROUP_META            (test_group_meta_get_type ())

#define TEST_GROUP(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), TEST_TYPE_GROUP, TestGroup))
#define TEST_IS_GROUP(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TEST_TYPE_GROUP))

#define TEST_GROUP_META(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TEST_TYPE_GROUP_META, TestGroupMeta))
#define TEST_IS_GROUP_META(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TEST_TYPE_GROUP_META))

typedef struct _ClutterActor            TestGroup;
typedef struct _ClutterActorClass       TestGroupClass;

typedef struct _TestGroupMeta {
  ClutterChildMeta parent_instance;

  guint is_focus : 1;
} TestGroupMeta;

typedef struct _ClutterChildMetaClass   TestGroupMetaClass;

GType test_group_meta_get_type (void);

G_DEFINE_TYPE (TestGroupMeta, test_group_meta, CLUTTER_TYPE_CHILD_META)

enum
{
  PROP_META_0,

  PROP_META_FOCUS
};

static void
test_group_meta_set_property (GObject      *gobject,
                              guint         prop_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
  TestGroupMeta *self = TEST_GROUP_META (gobject);

  switch (prop_id)
    {
    case PROP_META_FOCUS:
      self->is_focus = g_value_get_boolean (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
test_group_meta_get_property (GObject    *gobject,
                              guint       prop_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
  TestGroupMeta *self = TEST_GROUP_META (gobject);

  switch (prop_id)
    {
    case PROP_META_FOCUS:
      g_value_set_boolean (value, self->is_focus);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
test_group_meta_class_init (TestGroupMetaClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GParamSpec *pspec;

  gobject_class->set_property = test_group_meta_set_property;
  gobject_class->get_property = test_group_meta_get_property;

  pspec = g_param_spec_boolean ("focus", NULL, NULL,
                                FALSE,
                                G_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, PROP_META_FOCUS, pspec);
}

static void
test_group_meta_init (TestGroupMeta *meta)
{
  meta->is_focus = FALSE;
}

static void
clutter_container_iface_init (ClutterContainerIface *iface)
{
  iface->child_meta_type = TEST_TYPE_GROUP_META;
}

GType test_group_get_type (void);

G_DEFINE_TYPE_WITH_CODE (TestGroup, test_group, CLUTTER_TYPE_ACTOR,
                         G_IMPLEMENT_INTERFACE (CLUTTER_TYPE_CONTAINER,
                                                clutter_container_iface_init))

static void
test_group_class_init (TestGroupClass *klass)
{
}

static void
test_group_init (TestGroup *self)
{
}

static void
script_child (void)
{
  ClutterScript *script = clutter_script_new ();
  GObject *container, *actor;
  GError *error = NULL;
  gboolean focus_ret;
  gchar *test_file;

  test_file = g_test_build_filename (G_TEST_DIST, "scripts", "test-script-child.json", NULL);
  clutter_script_load_from_file (script, test_file, &error);
  if (!g_test_quiet () && error)
    g_print ("Error: %s", error->message);

  g_assert_no_error (error);

  container = actor = NULL;
  clutter_script_get_objects (script,
                              "test-group", &container,
                              "test-rect-1", &actor,
                              NULL);
  g_assert (TEST_IS_GROUP (container));
  g_assert (CLUTTER_IS_ACTOR (actor));

  focus_ret = FALSE;
  clutter_container_child_get (CLUTTER_CONTAINER (container),
                               CLUTTER_ACTOR (actor),
                               "focus", &focus_ret,
                               NULL);
  g_assert (focus_ret);

  actor = clutter_script_get_object (script, "test-rect-2");
  g_assert (CLUTTER_IS_ACTOR (actor));

  focus_ret = FALSE;
  clutter_container_child_get (CLUTTER_CONTAINER (container),
                               CLUTTER_ACTOR (actor),
                               "focus", &focus_ret,
                               NULL);
  g_assert (!focus_ret);

  g_object_unref (script);
  g_free (test_file);
}

static void
script_single (void)
{
  ClutterScript *script = clutter_script_new ();
  ClutterColor color = { 0, };
  GObject *actor = NULL;
  GError *error = NULL;
  ClutterActor *rect;
  gchar *test_file;

  test_file = g_test_build_filename (G_TEST_DIST, "scripts", "test-script-single.json", NULL);
  clutter_script_load_from_file (script, test_file, &error);
  if (!g_test_quiet () && error)
    g_print ("Error: %s", error->message);

  g_assert_no_error (error);

  actor = clutter_script_get_object (script, "test");
  g_assert (CLUTTER_IS_ACTOR (actor));

  rect = CLUTTER_ACTOR (actor);
  g_assert_cmpfloat (clutter_actor_get_width (rect), ==, 50.0);
  g_assert_cmpfloat (clutter_actor_get_y (rect), ==, 100.0);

  clutter_actor_get_background_color (rect, &color);
  g_assert_cmpint (color.red, ==, 255);
  g_assert_cmpint (color.green, ==, 0xcc);
  g_assert_cmpint (color.alpha, ==, 0xff);

  g_object_unref (script);
  g_free (test_file);
}

static void
script_object_property (void)
{
  ClutterScript *script = clutter_script_new ();
  ClutterLayoutManager *manager;
  GObject *actor = NULL;
  GError *error = NULL;
  gchar *test_file;

  test_file = g_test_build_filename (G_TEST_DIST, "scripts", "test-script-object-property.json", NULL);
  clutter_script_load_from_file (script, test_file, &error);
  if (!g_test_quiet () && error)
    g_print ("Error: %s", error->message);

  g_assert_no_error (error);

  actor = clutter_script_get_object (script, "test");
  g_assert (CLUTTER_IS_ACTOR (actor));

  manager = clutter_actor_get_layout_manager (CLUTTER_ACTOR (actor));
  g_assert (CLUTTER_IS_BIN_LAYOUT (manager));

  g_object_unref (script);
  g_free (test_file);
}

static void
script_named_object (void)
{
  ClutterScript *script = clutter_script_new ();
  ClutterLayoutManager *manager;
  GObject *actor = NULL;
  GError *error = NULL;
  gchar *test_file;

  test_file = g_test_build_filename (G_TEST_DIST, "scripts", "test-script-named-object.json", NULL);
  clutter_script_load_from_file (script, test_file, &error);
  if (!g_test_quiet () && error)
    g_print ("Error: %s", error->message);

  g_assert_no_error (error);

  actor = clutter_script_get_object (script, "test");
  g_assert (CLUTTER_IS_ACTOR (actor));

  manager = clutter_actor_get_layout_manager (CLUTTER_ACTOR (actor));
  g_assert (CLUTTER_IS_BOX_LAYOUT (manager));
  g_assert (clutter_box_layout_get_orientation (CLUTTER_BOX_LAYOUT (manager)) == CLUTTER_ORIENTATION_VERTICAL);

  g_object_unref (script);
  g_free (test_file);
}

static void
script_margin (void)
{
  ClutterScript *script = clutter_script_new ();
  ClutterActor *actor;
  gchar *test_file;
  GError *error = NULL;

  test_file = g_test_build_filename (G_TEST_DIST, "scripts", "test-script-margin.json", NULL);
  clutter_script_load_from_file (script, test_file, &error);
  if (!g_test_quiet () && error)
    g_print ("Error: %s", error->message);

  g_assert_no_error (error);

  actor = CLUTTER_ACTOR (clutter_script_get_object (script, "actor-1"));
  g_assert_cmpfloat (clutter_actor_get_margin_top (actor), ==, 10.0f);
  g_assert_cmpfloat (clutter_actor_get_margin_right (actor), ==, 10.0f);
  g_assert_cmpfloat (clutter_actor_get_margin_bottom (actor), ==, 10.0f);
  g_assert_cmpfloat (clutter_actor_get_margin_left (actor), ==, 10.0f);

  actor = CLUTTER_ACTOR (clutter_script_get_object (script, "actor-2"));
  g_assert_cmpfloat (clutter_actor_get_margin_top (actor), ==, 10.0f);
  g_assert_cmpfloat (clutter_actor_get_margin_right (actor), ==, 20.0f);
  g_assert_cmpfloat (clutter_actor_get_margin_bottom (actor), ==, 10.0f);
  g_assert_cmpfloat (clutter_actor_get_margin_left (actor), ==, 20.0f);

  actor = CLUTTER_ACTOR (clutter_script_get_object (script, "actor-3"));
  g_assert_cmpfloat (clutter_actor_get_margin_top (actor), ==, 10.0f);
  g_assert_cmpfloat (clutter_actor_get_margin_right (actor), ==, 20.0f);
  g_assert_cmpfloat (clutter_actor_get_margin_bottom (actor), ==, 30.0f);
  g_assert_cmpfloat (clutter_actor_get_margin_left (actor), ==, 20.0f);

  actor = CLUTTER_ACTOR (clutter_script_get_object (script, "actor-4"));
  g_assert_cmpfloat (clutter_actor_get_margin_top (actor), ==, 10.0f);
  g_assert_cmpfloat (clutter_actor_get_margin_right (actor), ==, 20.0f);
  g_assert_cmpfloat (clutter_actor_get_margin_bottom (actor), ==, 30.0f);
  g_assert_cmpfloat (clutter_actor_get_margin_left (actor), ==, 40.0f);

  g_object_unref (script);
  g_free (test_file);
}

CLUTTER_TEST_SUITE (
  CLUTTER_TEST_UNIT ("/script/single-object", script_single)
  CLUTTER_TEST_UNIT ("/script/container-child", script_child)
  CLUTTER_TEST_UNIT ("/script/named-object", script_named_object)
  CLUTTER_TEST_UNIT ("/script/object-property", script_object_property)
  CLUTTER_TEST_UNIT ("/script/actor-margin", script_margin)
)
