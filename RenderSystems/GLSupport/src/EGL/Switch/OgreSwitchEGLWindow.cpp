/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/

#include "OgreRoot.h"
#include "OgreException.h"

#include "OgreSwitchEGLSupport.h"
#include "OgreSwitchEGLWindow.h"
#include "OgreViewport.h"

#include <switch.h>

namespace Ogre {
    SwitchEGLWindow::SwitchEGLWindow(SwitchEGLSupport *glsupport)
        : EGLWindow(glsupport)
    {
        mClosed = true;
    }

    void SwitchEGLWindow::getLeftAndTopFromNativeWindow( int & left, int & top, uint width, uint height )
    {
        // We are always fixed at (0, 0) for the Nintendo Switch.
        left = top = 0;
    }

    void SwitchEGLWindow::initNativeCreatedWindow(const NameValuePairList *miscParams)
    {
    }

    void SwitchEGLWindow::createNativeWindow( int &left, int &top, uint &width, uint &height, String &title )
    {
    }

    void SwitchEGLWindow::reposition( int left, int top )
    {
    }

    void SwitchEGLWindow::resize(uint width, uint height)
    {
    }

    void SwitchEGLWindow::windowMovedOrResized()
    {
        switch(appletGetOperationMode())
        {
            default:
            case AppletOperationMode_Handheld:
                LogManager::getSingletonPtr()->logMessage("Switched to handheld mode.");
                mWidth = 1280;
                mHeight = 720;
                break;
            case AppletOperationMode_Docked:
                LogManager::getSingletonPtr()->logMessage("Switched to docked mode.");
                mWidth = 1920;
                mHeight = 1080;
                break;
        }

        nwindowSetCrop(mWindow, 0, 0, mWidth, mHeight);

        // Notify viewports of resize
        ViewportList::iterator it = mViewportList.begin();
        while( it != mViewportList.end() )
            (*it++).second->_updateDimensions();
    }

    void SwitchEGLWindow::switchFullScreen(bool fullscreen)
    {

    }

    void SwitchEGLWindow::create(const String& name, uint width, uint height,
                               bool fullScreen, const NameValuePairList *miscParams)
    {
        // Most of these are fixed for the Nintendo Switch.
        mName = name;
        mLeft = 0;
        mTop = 0;
        mIsFullScreen = true;
        mIsExternal = false;
        mHwGamma = false;

        // Create the window and GL context.
        _notifySurfaceDestroyed();
        _notifySurfaceCreated(nwindowGetDefault(), 0);
    }

    void SwitchEGLWindow::_notifySurfaceDestroyed()
    {
        if(mClosed)
            return;

        mContext->setCurrent();
        mContext->_destroyInternalResources();

        eglDestroySurface(mEglDisplay, mEglSurface);
        EGL_CHECK_ERROR
        mEglSurface = 0;

        eglTerminate(mEglDisplay);
        EGL_CHECK_ERROR
        mEglDisplay = 0;

        mActive = false;
        mVisible = false;
        mClosed = true;
    }

    void SwitchEGLWindow::_notifySurfaceCreated(void* window, void* config)
    {
        // Set the new window.
        mWindow = window;

        nwindowSetDimensions((NWindow*)mWindow, mWidth, mHeight);

        mEglDisplay = mGLSupport->getGLDisplay();

        EGLint numConfigs;
        static const EGLint framebufferAttributeList[] =
        {
            EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
            EGL_RED_SIZE,     8,
            EGL_GREEN_SIZE,   8,
            EGL_BLUE_SIZE,    8,
            EGL_ALPHA_SIZE,   8,
            EGL_DEPTH_SIZE,   24,
            EGL_STENCIL_SIZE, 8,
            EGL_NONE
        };

        eglChooseConfig(mEglDisplay, framebufferAttributeList, &mEglConfig,
            1, &numConfigs);

        mEglSurface = eglCreateWindowSurface(mEglDisplay, mEglConfig,
            mWindow, nullptr);

        mContext = createEGLContext();
        mContext->setCurrent();

        // Crop and adjust the window size.
        windowMovedOrResized();

        mActive = true;
        mVisible = true;
        mClosed = false;
    }
}
