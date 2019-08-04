#ifndef CLIPBOARDCONTAINER_H
#define CLIPBOARDCONTAINER_H
#include <QList>
#include <QWeakPointer>
#include <typeindex>
#include "smartPointers/sharedpointerdefs.h"
#include "basicreadwrite.h"
#include "Animators/SmartPath/smartpath.h"
class QrealAnimator;
class BoundingBox;
class ContainerBox;
class Key;
class Animator;
class Property;

typedef QPair<qptr<Animator>, QByteArray> AnimatorKeyDataPair;

enum class ClipboardType : short {
    misc,
    boxes,
    keys,
    property,
    dynamicProperties,
    smartPath
};

class Clipboard : public StdSelfRef {
public:
    Clipboard(const ClipboardType type);
    ClipboardType getType() const;
protected:
    QByteArray mData;
private:
    const ClipboardType mType;
};

class SmartPathClipboard : public Clipboard {
public:
    SmartPathClipboard(const SmartPath& path) :
        Clipboard(ClipboardType::smartPath), mPath(path) {}

    const SmartPath& path() const { return mPath; }
private:
    SmartPath mPath;
};

class BoxesClipboard : public Clipboard {
    friend class StdSelfRef;
protected:
    BoxesClipboard(const QList<BoundingBox*> &src);
public:
    void pasteTo(ContainerBox * const parent);
};

class KeysClipboard : public Clipboard {
    friend class StdSelfRef;
protected:
    KeysClipboard();
public:
    void paste(const int pasteFrame, const bool merge,
               const std::function<void(Key*)> &selectAction = nullptr);

    void addTargetAnimator(Animator *anim, const QByteArray& keyData);
private:
    QList<AnimatorKeyDataPair> mAnimatorData;
};


class PropertyClipboard : public Clipboard {
    friend class StdSelfRef;
protected:
    PropertyClipboard(const Property * const source);
public:
    bool paste(Property * const target);

    template<typename T>
    bool hasType() {
        return mContentType == std::type_index(typeid(T));
    }

    template<typename T>
    bool compatibleTarget(T* const obj) const {
        return mContentType == std::type_index(typeid(obj));
    }
private:
    const std::type_index mContentType;
};

template <typename T>
class DynamicComplexAnimatorBase;

class DynamicPropsClipboard : public Clipboard {
    friend class StdSelfRef;
protected:
    template<typename T>
    DynamicPropsClipboard(const QList<T*>& source) :
        Clipboard(ClipboardType::dynamicProperties),
        mContentBaseType(std::type_index(typeid(T))) {
        QBuffer dst(&mData);
        dst.open(QIODevice::WriteOnly);
        const int count = source.count();
        dst.write(rcConstChar(&count), sizeof(int));
        for(const auto& src : source)
            src->writeProperty(&dst);
        dst.close();
    }
public:
    template<typename T>
    bool paste(DynamicComplexAnimatorBase<T> * const target) {
        if(!compatibleTarget(target)) return false;
        QBuffer src(&mData);
        src.open(QIODevice::ReadOnly);
        target->readProperty(&src);
        src.close();
        return true;
    }

    template<typename T>
    bool compatibleTarget(DynamicComplexAnimatorBase<T> * const obj) {
        Q_UNUSED(obj);
        return mContentBaseType == std::type_index(typeid(T));
    }
private:
    const std::type_index mContentBaseType;
};

#endif // CLIPBOARDCONTAINER_H