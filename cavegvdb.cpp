#include <omega.h>
#include <omegaGl.h>
#include <iostream>
#include <vector>

#include "app.h"

using namespace std;
using namespace omega;

///////////////////////////////////////////////////////////////////////////////
class GVDBRenderModule : public EngineModule
{
public:
    GVDBRenderModule() :
        EngineModule("GVDBRenderModule"), visible(true), app(0), initalized(false)
    {

    }

    virtual void initializeRenderer(Renderer* r);

    virtual void update(const UpdateContext& context)
    {
        // After a frame all render passes had a chance to update their
        // textures. reset the raster update flag.       
    }
    
    virtual void dispose()
    {

    }

    void initGvdb(const string& inifile) {
        app = new GvdbApp();
    }

    bool visible;
    bool initalized;
    GvdbApp* app;
};

///////////////////////////////////////////////////////////////////////////////
class GVDBRenderPass : public RenderPass
{
public:
    GVDBRenderPass(Renderer* client, GVDBRenderModule* prm) : 
        RenderPass(client, "GVDBRenderPass"), 
        module(prm) {}
    
    virtual void initialize()
    {
        RenderPass::initialize();
    }

    virtual void render(Renderer* client, const DrawContext& context)
    {
    	if(context.task == DrawContext::SceneDrawTask)
        {
            glPushAttrib(GL_TEXTURE_BIT | GL_ENABLE_BIT | GL_CURRENT_BIT);
            client->getRenderer()->beginDraw3D(context);

            if(!module->initalized && module->app) {
                module->app->init(context.viewport.width(), context.viewport.height());
                module->initalized = true;
            }
	
    	    if(module->initalized && module->visible)
    	    { 
                Vector3f cp = context.camera->getPosition();
                float campos[3] = {cp[0], cp[1], cp[2]};
                float* MV = context.modelview.cast<float>().data();
                float* P = context.projection.cast<float>().data();
                module->app->display(MV, P, campos);

                if(oglError) return;
    	    }
            
            client->getRenderer()->endDraw();
            glPopAttrib();
        }
        
    }

private:
    GVDBRenderModule* module;

};

///////////////////////////////////////////////////////////////////////////////
void GVDBRenderModule::initializeRenderer(Renderer* r)
{
    r->addRenderPass(new GVDBRenderPass(r, this));
}

///////////////////////////////////////////////////////////////////////////////
GVDBRenderModule* initialize()
{
    GVDBRenderModule* prm = new GVDBRenderModule();
    ModuleServices::addModule(prm);
    prm->doInitialize(Engine::instance());
    return prm;
}

///////////////////////////////////////////////////////////////////////////////
// Python API
#include "omega/PythonInterpreterWrapper.h"
BOOST_PYTHON_MODULE(cavegvdb)
{
    //
    PYAPI_REF_BASE_CLASS(GVDBRenderModule)
    PYAPI_METHOD(GVDBRenderModule, initGvdb)
    ;

    def("initialize", initialize, PYAPI_RETURN_REF);
}