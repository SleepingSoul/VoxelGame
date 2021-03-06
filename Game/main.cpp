#include <iostream>
#include <Engine/Engine.h>
#include <Engine/RenderGraph/AcquireNode.h>
#include <Engine/RenderGraph/PresentNode.h>
#include <Engine/CameraSystem.h>
#include <entt/entt.hpp>

#include "FrameRateCounter.h"
#include "Renderer.h"
#include "FreeCam.h"
#include "Chunk.h"
#include "ChunkMesh.h"
#include "ChunkUpdater.h"
#include "ChunkMesher.h"
#include "ChunkManager.h"
#include "TextureManager.h"
#include "BlockManager.h"
#include "TerrainGenerator.h"
#include "SkyboxManager.h"

int main() {
    VoxelEngine::Engine engine;
    VoxelEngine::Window window(800, 600, "VoxelGame");
    engine.addWindow(window);

    VoxelEngine::Graphics& graphics = engine.getGraphics();
    graphics.pickPhysicalDevice(window);

    VoxelEngine::RenderGraph renderGraph(graphics.device(), 2);
    engine.setRenderGraph(renderGraph);

    FrameRateCounter counter(window, "VoxelGame");
    engine.getUpdateGroup().add(counter, 0);

    VoxelEngine::Camera camera(engine, window.getFramebufferWidth(), window.getFramebufferHeight(), glm::radians(90.0f), 0.01f, 1000.0f);
    VoxelEngine::CameraSystem cameraSystem(engine);
    cameraSystem.setCamera(camera);
    engine.getUpdateGroup().add(cameraSystem, 90);

    window.onFramebufferResized().connect<&VoxelEngine::Camera::setSize>(&camera);

    FreeCam freeCam(camera, window.input());
    engine.getUpdateGroup().add(freeCam, 10);

    freeCam.setPosition({ 0, 80, 0 });

    SkyboxManager skyboxManager(engine, cameraSystem);
    TextureManager textureManager(engine);
    BlockManager blockManager;
    World world;

    ChunkManager chunkManager(world, freeCam, 16);
    engine.getUpdateGroup().add(chunkManager, 20);

    TerrainGenerator terrainGenerator(world, chunkManager);
    terrainGenerator.run();

    ChunkUpdater chunkUpdater(engine, world, blockManager, chunkManager);
    chunkUpdater.run();

    ChunkMesher chunkMesher(engine, world, blockManager);
    engine.getUpdateGroup().add(chunkMesher, 30);
    chunkMesher.run();

    chunkManager.setTerrainGenerator(terrainGenerator);
    chunkManager.setChunkUpdater(chunkUpdater);
    chunkManager.setChunkMesher(chunkMesher);

    Renderer renderer(engine, renderGraph, cameraSystem, world, textureManager, skyboxManager);
    engine.getUpdateGroup().add(renderer, 100);

    cameraSystem.setTransferNode(renderer.transferNode());
    chunkMesher.setTransferNode(renderer.transferNode());
    textureManager.createTexture(renderer.transferNode(), renderer.mipmapGenerator());
    skyboxManager.transfer(renderer.transferNode());
    skyboxManager.createPipeline(renderer.chunkRenderer().renderPass());

    engine.run();

    terrainGenerator.stop();
    chunkUpdater.stop();
    chunkMesher.stop();
    renderer.wait();

    return 0;
}