
#ifndef GLMARK2_NATIVE_STATE_NEXUS_H_
#define GLMARK2_NATIVE_STATE_NEXUS_H_


#include "native-state.h"

#include "nexus_platform.h"
#include "nexus_display.h"
#include "nexus_core_utils.h"

#include "nxpl/default_nexus.h"

class NativeStateNexus : public NativeState
{
public:
    NativeStateNexus();
    ~NativeStateNexus();
    virtual bool init_display();
    virtual void* display();
    virtual bool create_window(WindowProperties const& properties);
    virtual void* window(WindowProperties& properties);
    virtual void visible(bool v);
    virtual bool should_quit();
    virtual void flip();

private:
    void InitHDMIOutput(NEXUS_DisplayHandle display);
    NEXUS_DisplayHandle gs_nexus_display;
    NXPL_PlatformHandle  nxpl_handle;
    void * nexus_window;
};

#endif /* GLMARK2_NATIVE_STATE_NEXUS_H_ */
