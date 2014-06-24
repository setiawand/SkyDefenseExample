#include "AppDelegate.h"
#include "HelloWorldScene.h"
#include "SimpleAudioEngine.h"

USING_NS_CC;

using namespace CocosDenshion;

AppDelegate::AppDelegate() {

}

AppDelegate::~AppDelegate() {
}

bool AppDelegate::applicationDidFinishLaunching() {
	// initialize director
	auto director = Director::getInstance();
	auto glview = director->getOpenGLView();
	if (!glview) {
		glview = GLView::create("My Game");
		director->setOpenGLView(glview);
	}
	Size screenSize = glview->getFrameSize();
	Size designSize = Size(2048, 1536);

	glview->setDesignResolutionSize(designSize.width, designSize.height, ResolutionPolicy::EXACT_FIT);

	auto fileUtils = FileUtils::getInstance();
	std::vector<std::string> searchPaths;

	if (screenSize.height > 768) {
		searchPaths.push_back("ipadhd");
	} else if (screenSize.height > 320) {
		searchPaths.push_back("ipad");
	} else {
		searchPaths.push_back("iphone");
	}
	fileUtils->setSearchPaths(searchPaths);

	director->setContentScaleFactor(screenSize.height/designSize.height);

	//preload sound effects and background music
	SimpleAudioEngine::getInstance()->preloadBackgroundMusic("background.mp3");
	SimpleAudioEngine::getInstance()->preloadEffect("bombFail.wav");
	SimpleAudioEngine::getInstance()->preloadEffect("bombRelease.wav");
	SimpleAudioEngine::getInstance()->preloadEffect("boom.wav");
	SimpleAudioEngine::getInstance()->preloadEffect("health.wav");
	SimpleAudioEngine::getInstance()->preloadEffect("fire_truck.wav");

	//lower playback volume for effects
	SimpleAudioEngine::getInstance()->setEffectsVolume(0.4f);

	// turn on display FPS
	director->setDisplayStats(false);

	// set FPS. the default value is 1.0/60 if you don't call this
	director->setAnimationInterval(1.0 / 60);

	// create a scene. it's an autorelease object
	auto scene = HelloWorld::createScene();

	// run
	director->runWithScene(scene);

	return true;
}

// This function will be called when the app is inactive. When comes a phone call,it's be invoked too
void AppDelegate::applicationDidEnterBackground() {
	Director::getInstance()->stopAnimation();
	Director::getInstance()->pause();

	// if you use SimpleAudioEngine, it must be pause
	// SimpleAudioEngine::getInstance()->pauseBackgroundMusic();
}

// this function will be called when the app is active again
void AppDelegate::applicationWillEnterForeground() {
	Director::getInstance()->startAnimation();
	Director::getInstance()->resume();

	// if you use SimpleAudioEngine, it must resume here
	// SimpleAudioEngine::getInstance()->resumeBackgroundMusic();
}
