
#include "native-state-nexus.h"
#include "log.h"

NativeStateNexus::NativeStateNexus()
{
}

NativeStateNexus::~NativeStateNexus()
{
    if (gs_nexus_display != 0)
    {
        NXPL_UnregisterNexusDisplayPlatform(nxpl_handle);
        NEXUS_Display_Close(gs_nexus_display);
    }

    if (nexus_window)
    {
        NXPL_DestroyNativeWindow(nexus_window);
    }

    NEXUS_Platform_Uninit();
}

static void hotplug_callback(void *pParam, int iParam)
{
    NEXUS_HdmiOutputStatus status;
    NEXUS_HdmiOutputHandle hdmi = (NEXUS_HdmiOutputHandle)pParam;
    NEXUS_DisplayHandle display = (NEXUS_DisplayHandle)iParam;

    NEXUS_HdmiOutput_GetStatus(hdmi, &status);
    Log::debug("HDMI hotplug event: %s", status.connected?"connected":"not connected");

   /* the app can choose to switch to the preferred format, but it's not required. */
    if (status.connected)
    {
        NEXUS_DisplaySettings displaySettings;
        NEXUS_Display_GetSettings(display, &displaySettings);
        Log::debug("Switching to preferred format %d", status.preferredVideoFormat);
        displaySettings.format = status.preferredVideoFormat;
        NEXUS_Display_SetSettings(display, &displaySettings);
    }
}

void NativeStateNexus::InitHDMIOutput(NEXUS_DisplayHandle display)
{
    Log::debug("%s", __PRETTY_FUNCTION__);
    NEXUS_HdmiOutputSettings      hdmiSettings;
    NEXUS_PlatformConfiguration   platform_config;
    NEXUS_Platform_GetConfiguration(&platform_config);

    if (platform_config.outputs.hdmi[0])
    {
        NEXUS_Display_AddOutput(display, NEXUS_HdmiOutput_GetVideoConnector(platform_config.outputs.hdmi[0]));
        /* Install hotplug callback -- video only for now */
        NEXUS_HdmiOutput_GetSettings(platform_config.outputs.hdmi[0], &hdmiSettings);
        hdmiSettings.hotplugCallback.callback = hotplug_callback;
        hdmiSettings.hotplugCallback.context = platform_config.outputs.hdmi[0];
        hdmiSettings.hotplugCallback.param = (int)display;
        NEXUS_HdmiOutput_SetSettings(platform_config.outputs.hdmi[0], &hdmiSettings);

        /* Force a hotplug to switch to a supported format if necessary */
        hotplug_callback(platform_config.outputs.hdmi[0], (int)display);
    }
}

bool NativeStateNexus::init_display()
{
    NEXUS_PlatformSettings      platform_settings;
    NEXUS_Error                 err;
    NEXUS_DisplaySettings       display_settings;
    NEXUS_GraphicsSettings      graphics_settings;


    NEXUS_Platform_GetDefaultSettings(&platform_settings);
    platform_settings.openFrontend = false;

    err = NEXUS_Platform_Init(&platform_settings);
    if (err)
    {
        Log::error("Err: NEXUS_Platform_Init() failed");
        return false;
    }
    NEXUS_Display_GetDefaultSettings(&display_settings);

    display_settings.format = NEXUS_VideoFormat_e1080p;
    gs_nexus_display = NEXUS_Display_Open(0, &display_settings);
    if (gs_nexus_display == NULL)
    {
        NEXUS_Platform_Uninit();
        Log::error("Err: OpenDisplay() failed");
        return false;
    }
    InitHDMIOutput(gs_nexus_display);

    NXPL_RegisterNexusDisplayPlatform(&nxpl_handle, gs_nexus_display);

    NEXUS_Display_GetGraphicsSettings(gs_nexus_display, &graphics_settings);
    graphics_settings.visible = true;
    graphics_settings.horizontalFilter = NEXUS_GraphicsFilterCoeffs_eBilinear;
    graphics_settings.verticalFilter = NEXUS_GraphicsFilterCoeffs_eBilinear;

    /* Disable blend with video plane */
    graphics_settings.sourceBlendFactor = NEXUS_CompositorBlendFactor_eOne;
    graphics_settings.destBlendFactor   = NEXUS_CompositorBlendFactor_eZero;
    NEXUS_Display_SetGraphicsSettings(gs_nexus_display, &graphics_settings);
    return true;
}

void* NativeStateNexus::display()
{
    return EGL_DEFAULT_DISPLAY;
}

bool NativeStateNexus::create_window(WindowProperties const& /*properties*/)
{
    NXPL_NativeWindowInfo   win_info;
    win_info.x = 0;
    win_info.y = 0;
    //win_info.width = properties.width;
    //win_info.height = properties.height;
    win_info.width = 1920;
    win_info.height = 1080;
    //if (properties.width == 1920 && properties.height == 1080)
        win_info.stretch = false;

    nexus_window = NXPL_CreateNativeWindow(&win_info);

    return nexus_window;
}

void* NativeStateNexus::window(WindowProperties& properties)
{
    properties.width = 1920;
    properties.height = 1080;
    return nexus_window;
}

void NativeStateNexus::visible(bool /*v*/)
{

}

bool NativeStateNexus::should_quit()
{
    return false;
}

void NativeStateNexus::flip()
{

}
