/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * DESIGN
 * ------
 *
 * The purpose of the jsapi library is to offer a first-class JavaScript interface to the core
 * Filament classes: Engine, Renderer, Texture, etc.
 *
 * Emscripten offers two ways to binding JavaScript to C++: embind and WebIDL. We chose embind.
 *
 * With WebIDL, we would need to author WebIDL files and generate opaque C++ from the IDL, which
 * complicates the build process and ultimately results in the same amount of code. Using embind is
 * more direct and controllable.
 *
 * For nested classes, we use $ as the separator character because embind does not support nested
 * classes and it would transform dot separators into $ anyway. By using $ here, we at least make
 * this explicit rather than mysterious.
 */

#include <filament/Camera.h>
#include <filament/Engine.h>
#include <filament/IndexBuffer.h>
#include <filament/RenderableManager.h>
#include <filament/Renderer.h>
#include <filament/Scene.h>
#include <filament/SwapChain.h>
#include <filament/VertexBuffer.h>
#include <filament/View.h>

#include <utils/EntityManager.h>

#include <emscripten/bind.h>

using namespace emscripten;

namespace {

// For convenience, declare terse private aliases to nested types. This lets us avoid extremely
// verbose binding declarations.
using RenderBuilder = filament::RenderableManager::Builder;
using VertexBuilder = filament::VertexBuffer::Builder;
using IndexBuilder = filament::IndexBuffer::Builder;

}

namespace filaweb {

struct Engine {
    static Engine* create() {
        return new Engine { filament::Engine::create(filament::Engine::Backend::OPENGL) };
    }
    static void destroy(Engine* engine) {
        filament::Engine::destroy(&engine->engine);
    }

    // SwapChain
    filament::SwapChain* createSwapChain() {
        return engine->createSwapChain(nullptr);
    }
    void destroySwapChain(filament::SwapChain* sc) {
        engine->destroy(sc);
    };

    // Renderer
    filament::Renderer* createRenderer() {
        return engine->createRenderer();
    }
    void destroyRenderer(filament::Renderer* renderer) {
        engine->destroy(renderer);
    };

    // View
    filament::View* createView() {
        return engine->createView();
    }
    void destroyView(filament::View* view) {
        engine->destroy(view);
    };

    // Scene
    filament::Scene* createScene() {
        return engine->createScene();
    }
    void destroyScene(filament::Scene* scene) {
        engine->destroy(scene);
    };

    // Camera
    filament::Camera* createCamera() {
        return engine->createCamera();
    }
    void destroyCamera(filament::Camera* camera) {
        engine->destroy(camera);
    };

    void destroyEntity(utils::Entity entity) {
        engine->destroy(entity);
    };

    void destroyVertexBuffer(filament::VertexBuffer* vb) {
        engine->destroy(vb);
    };

    filament::Engine* engine;
};

EMSCRIPTEN_BINDINGS(value_types) {

    value_array<math::float3>("float3")
        .element(&math::float3::x)
        .element(&math::float3::y)
        .element(&math::float3::z);

    value_array<filament::Box>("Box")
        .element(&filament::Box::center)
        .element(&filament::Box::halfExtent);

}

EMSCRIPTEN_BINDINGS(core_types) {

    //
    // Engine
    //
    class_<Engine>("Engine")
        .class_function("create", &Engine::create, allow_raw_pointers())
        .class_function("destroy", &Engine::destroy, allow_raw_pointers())

        .function("createSwapChain", &Engine::createSwapChain, allow_raw_pointers())
        .function("destroySwapChain", &Engine::destroySwapChain, allow_raw_pointers())

        .function("createRenderer", &Engine::createRenderer, allow_raw_pointers())
        .function("destroyRenderer", &Engine::destroyRenderer, allow_raw_pointers())

        .function("createView", &Engine::createView, allow_raw_pointers())
        .function("destroyView", &Engine::destroyView, allow_raw_pointers())

        .function("createScene", &Engine::createScene, allow_raw_pointers())
        .function("destroyScene", &Engine::destroyScene, allow_raw_pointers())

        .function("createCamera", &Engine::createCamera, allow_raw_pointers())
        .function("destroyCamera", &Engine::destroyCamera, allow_raw_pointers())

        .function("destroyEntity", &Engine::destroyEntity)
        .function("destroyVertexBuffer", &Engine::destroyVertexBuffer, allow_raw_pointers());

    //
    // SwapChain
    //
    class_<filament::SwapChain>("SwapChain");

    //
    // Renderer
    //
    class_<filament::Renderer>("Renderer")
        .function("render", &filament::Renderer::render, allow_raw_pointers());

    //
    // View
    //
    class_<filament::View>("View")
        .function("setScene", &filament::View::setScene, allow_raw_pointers())
        .function("setCamera", &filament::View::setCamera, allow_raw_pointers());

    //
    // Scene
    //
    class_<filament::Scene>("Scene")
        .function("addEntity", &filament::Scene::addEntity);

    //
    // Camera
    //
    class_<filament::Camera>("Camera");

    //
    // RenderableManager
    //
    class_<RenderBuilder>("RenderableManager$Builder")

        .function("build", (void (*)(RenderBuilder*, filaweb::Engine*, utils::Entity)) []
                (RenderBuilder* builder, filaweb::Engine* engine, utils::Entity entity) {
            builder->build(*engine->engine, entity);
        }, allow_raw_pointers())

        .function("boundingBox", (RenderBuilder* (*)(RenderBuilder*, filament::Box)) []
                (RenderBuilder* builder, filament::Box box) {
            return &builder->boundingBox(box);
        }, allow_raw_pointers())

        .function("culling", (RenderBuilder* (*)(RenderBuilder*, bool)) []
                (RenderBuilder* builder, bool enable) {
            return &builder->culling(enable);
        }, allow_raw_pointers())

        .function("receiveShadows", (RenderBuilder* (*)(RenderBuilder*, bool)) []
                (RenderBuilder* builder, bool enable) {
            return &builder->receiveShadows(enable);
        }, allow_raw_pointers())

        .function("castShadows", (RenderBuilder* (*)(RenderBuilder*, bool)) []
                (RenderBuilder* builder, bool enable) {
            return &builder->castShadows(enable);
        }, allow_raw_pointers());

    class_<filament::RenderableManager>("RenderableManager")
        .class_function("Builder", (RenderBuilder (*)(int)) []
            (int n) { return RenderBuilder(n); });

    //
    // VertexBuffer
    //
    class_<VertexBuilder>("VertexBuffer$Builder")

        .function("build", (void (*)(VertexBuilder*, filaweb::Engine*)) []
                (VertexBuilder* builder, filaweb::Engine* engine) {
            builder->build(*engine->engine);
        }, allow_raw_pointers())

        .function("vertexCount", (VertexBuilder* (*)(VertexBuilder*, int)) []
                (VertexBuilder* builder, int count) {
            return &builder->vertexCount(count);
        }, allow_raw_pointers())

        .function("bufferCount", (VertexBuilder* (*)(VertexBuilder*, int)) []
                (VertexBuilder* builder, int count) {
            return &builder->bufferCount(count);
        }, allow_raw_pointers());

    class_<filament::VertexBuffer>("VertexBuffer")
        .class_function("Builder", (VertexBuilder (*)()) [] () { return VertexBuilder(); });

    //
    // IndexBuffer
    //
    class_<IndexBuilder>("IndexBuffer$Builder")
        .function("build", (void (*)(IndexBuilder*, filaweb::Engine*)) []
                (IndexBuilder* builder, filaweb::Engine* engine) {
            builder->build(*engine->engine);
        }, allow_raw_pointers());

    class_<filament::IndexBuffer>("IndexBuffer")
        .class_function("Builder", (IndexBuilder (*)()) [] () { return IndexBuilder(); });

}

EMSCRIPTEN_BINDINGS(utils) {

    class_<utils::Entity>("Entity");

    class_<utils::EntityManager>("EntityManager")
        .class_function("get", (utils::EntityManager* (*)()) []
            () { return &utils::EntityManager::get(); }, allow_raw_pointers())
        .function("create", select_overload<utils::Entity()>(&utils::EntityManager::create))
        .function("destroy", select_overload<void(utils::Entity)>(&utils::EntityManager::destroy));
}

} // namespace filaweb
