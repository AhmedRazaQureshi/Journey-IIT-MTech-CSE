#include <gst/gst.h>
#include "video_conferencing.h" // Include my header file (if needed further)

#define WIDTH 640
#define HEIGHT 480
#define PORT "5000"              // Change this to the receiver's port no.
#define IP_ADDRESS "192.168.0.2" // Change this to the receiver's IP address

int main(int argc, char *argv[])
{
    GstElement *pipeline, *source, *filter, *encoder, *payloader, *sink;
    GstCaps *caps;
    GstBus *bus;
    GstMessage *msg;
    GstStateChangeReturn ret;

    // Initialize GStreamer
    gst_init(&argc, &argv);

    // Create the elements
    source = gst_element_factory_make("v4l2src", "source");
    filter = gst_element_factory_make("capsfilter", "filter");
    encoder = gst_element_factory_make("x264enc", "encoder");
    payloader = gst_element_factory_make("rtph264pay", "payloader");
    sink = gst_element_factory_make("udpsink", "sink");

    // Create the pipeline
    pipeline = gst_pipeline_new("video-conference-sender");

    if (!pipeline || !source || !filter || !encoder || !payloader || !sink)
    {
        g_printerr("One or more elements could not be created. Exiting.\n");
        return -1;
    }

    // Set source properties
    g_object_set(source, "device", "/dev/video0", NULL); // Change this to match your camera device
    g_object_set(filter, "caps", gst_caps_new_simple("video/x-raw", "width", G_TYPE_INT, WIDTH, "height", G_TYPE_INT, HEIGHT, NULL), NULL);

    // Set sink properties
    g_object_set(sink, "host", IP_ADDRESS, "port", PORT, NULL);

    // Add elements to the pipeline
    gst_bin_add_many(GST_BIN(pipeline), source, filter, encoder, payloader, sink, NULL);

    // Link elements
    if (!gst_element_link_many(source, filter, encoder, payloader, sink, NULL))
    {
        g_printerr("Elements could not be linked. Exiting.\n");
        gst_object_unref(pipeline);
        return -1;
    }

    // Set the pipeline to the playing state
    ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE)
    {
        g_printerr("Unable to set the pipeline to the playing state. Exiting.\n");
        gst_object_unref(pipeline);
        return -1;
    }

    // Wait until error or EOS
    bus = gst_element_get_bus(pipeline);
    msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE, GST_MESSAGE_ERROR | GST_MESSAGE_EOS);

    // Parse message
    if (msg != NULL)
    {
        GError *err;
        gchar *debug_info;

        switch (GST_MESSAGE_TYPE(msg))
        {
        case GST_MESSAGE_ERROR:
            gst_message_parse_error(msg, &err, &debug_info);
            g_printerr("Error received from element %s: %s\n", GST_OBJECT_NAME(msg->src), err->message);
            g_printerr("Debugging information: %s\n", debug_info ? debug_info : "none");
            g_clear_error(&err);
            g_free(debug_info);
            break;
        case GST_MESSAGE_EOS:
            g_print("End-Of-Stream reached.\n");
            break;
        default:
            // Should not be reached
            g_printerr("Unexpected message received.\n");
            break;
        }

        gst_message_unref(msg);
    }

    // Free resources
    gst_object_unref(bus);
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);

    return 0;
}
