#include "document.h"
#include "FileCacheHandlers/filecachehandler.h"
#include "canvas.h"
#include "simpletask.h"

Document* Document::sInstance = nullptr;

bool Document::FileCompare::operator()(const FileHandler &f1,
                                       const FileHandler &f2) {
    return f1->getFilePath() < f2->getFilePath();
}

Document::Document(AudioHandler &audioHandler) :
    fActions(*this), fRenderHandler(*this, audioHandler) {
    sInstance = this;
    connect(&fRenderHandler, &RenderHandler::queTasksAndUpdate,
            this, &Document::actionFinished);
    connect(&fTaskScheduler, &TaskScheduler::finishedAllQuedTasks,
            this, &Document::actionFinished);
}

void Document::actionFinished() {
    SimpleTask::sProcessAll();
    fTaskScheduler.queTasks();

    for(const auto& scene : fVisibleScenes) {
        if(scene.first->newUndoRedoSet())
            emit documentChanged();
        emit scene.first->requestUpdate();
    }
//    if(fActiveScene) {
//        if(fActiveScene->newUndoRedoSet())
//            emit documentChanged();
//        emit fActiveScene->requestUpdate();
//    }
}

void Document::replaceClipboard(const stdsptr<ClipboardContainer> &container) {
    fClipboardContainer = container;
}

ClipboardContainer *Document::getClipboardContainer(
        const ClipboardContainerType &type) {
    if(!fClipboardContainer) return nullptr;
    if(type == fClipboardContainer->getType())
        return fClipboardContainer.get();
    return nullptr;
}

PropertyClipboardContainer* Document::getPropertyClipboardContainer() {
    auto contT = getClipboardContainer(CCT_PROPERTY);
    return static_cast<PropertyClipboardContainer*>(contT);
}

BoxesClipboardContainer* Document::getBoxesClipboardContainer() {
    auto contT = getClipboardContainer(CCT_BOXES);
    return static_cast<BoxesClipboardContainer*>(contT);
}

void Document::setCanvasMode(const CanvasMode mode) {
    fCanvasMode = mode;
    emit canvasModeSet(mode);
}

Canvas *Document::createNewScene() {
    const auto newScene = SPtrCreate(Canvas)(*this);
    fScenes.append(newScene);
    SWT_addChild(newScene.get());
    emit sceneCreated(newScene.get());
    return newScene.get();
}

bool Document::removeScene(const qsptr<Canvas>& scene) {
    const int id = fScenes.indexOf(scene);
    return removeScene(id);
}

bool Document::removeScene(const int id) {
    if(id < 0 || id >= fScenes.count()) return false;
    const auto scene = fScenes.takeAt(id);
    SWT_removeChild(scene.data());
    emit sceneRemoved(scene.data());
    emit sceneRemoved(id);
    return true;
}

void Document::addVisibleScene(Canvas * const scene) {
    fVisibleScenes[scene]++;
}

bool Document::removeVisibleScene(Canvas * const scene) {
    const auto it = fVisibleScenes.find(scene);
    if(it == fVisibleScenes.end()) return false;
    if(it->second == 1) fVisibleScenes.erase(it);
    else it->second--;
    return true;
}

void Document::setActiveScene(Canvas * const scene) {
    if(scene == fActiveScene) return;
    if(fActiveScene) {
        disconnect(fActiveScene, nullptr, this, nullptr);
    }
    fActiveScene = scene;
    if(fActiveScene) {
        connect(fActiveScene, &Canvas::currentBoxChanged,
                this, &Document::currentBoxChanged);
        connect(fActiveScene, &Canvas::selectedPaintSettingsChanged,
                this, &Document::selectedPaintSettingsChanged);
        connect(fActiveScene, &Canvas::destroyed,
                this, &Document::clearActiveScene);
        emit currentBoxChanged(fActiveScene->getCurrentBox());
        emit selectedPaintSettingsChanged();
    }
    emit activeSceneSet(scene);
}

void Document::clearActiveScene() {
    setActiveScene(nullptr);
}

int Document::getActiveSceneFrame() const {
    if(!fActiveScene) return 0;
    return fActiveScene->anim_getCurrentAbsFrame();
}

void Document::setActiveSceneFrame(const int frame) {
    if(!fActiveScene) return;
    if(fActiveScene->anim_getCurrentRelFrame() == frame) return;
    fActiveScene->anim_setAbsFrame(frame);
    emit activeSceneFrameSet(frame);
}

void Document::incActiveSceneFrame() {
    setActiveSceneFrame(getActiveSceneFrame() + 1);
}

void Document::decActiveSceneFrame() {
    setActiveSceneFrame(getActiveSceneFrame() - 1);
}

Gradient *Document::createNewGradient() {
    const auto grad = SPtrCreate(Gradient)();
    fGradients.append(grad);
    emit gradientCreated(grad.get());
    return grad.get();
}

Gradient *Document::duplicateGradient(const int id) {
    if(id < 0 || id >= fGradients.count()) return nullptr;
    const auto from = fGradients.at(id).get();
    const auto newGrad = createNewGradient();
    QBuffer buffer;
    buffer.open(QIODevice::ReadWrite);
    from->write(-1, &buffer);
    if(buffer.reset()) newGrad->read(&buffer);
    buffer.close();
    return newGrad;
}

bool Document::removeGradient(const qsptr<Gradient> &gradient) {
    const int id = fGradients.indexOf(gradient);
    return removeGradient(id);
}

bool Document::removeGradient(const int id) {
    if(id < 0 || id >= fGradients.count()) return false;
    const auto grad = fGradients.takeAt(id);
    emit gradientRemoved(grad.data());
    return true;
}

void Document::clear() {
    const int nScenes = fScenes.count();
    for(int i = 0; i < nScenes; i++)
        removeScene(0);
    replaceClipboard(nullptr);
}

void Document::SWT_setupAbstraction(SWT_Abstraction * const abstraction,
                                    const UpdateFuncs &updateFuncs,
                                    const int visiblePartWidgetId) {
    for(const auto& scene : fScenes) {
        auto abs = scene->SWT_abstractionForWidget(updateFuncs,
                                                   visiblePartWidgetId);
        abstraction->addChildAbstraction(abs);
    }
}
