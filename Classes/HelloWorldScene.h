#ifndef __HELLOWORLD_SCENE_H__
#define __HELLOWORLD_SCENE_H__

#include "cocos2d.h"
#include "SimpleAudioEngine.h"

enum {
	kSpriteBomb,
	kSpriteShockwave,
	kSpriteMeteor,
	kSpriteHealth,
	kSpriteHalo,
	kSpriteSparkle
};

enum {
	kBackground, kMiddleground, kForeground
};

using namespace CocosDenshion;
using namespace cocos2d;

class HelloWorld: public cocos2d::Layer {
public:
	// there's no 'id' in cpp, so we recommend returning the class instance pointer
	static cocos2d::Scene* createScene();

	// Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
	virtual bool init();

	// a selector callback
	void menuCloseCallback(cocos2d::Ref* pSender);

	// implement the "static create()" method manually
	CREATE_FUNC(HelloWorld)
	;

	void fallingObjectDone(Node* pSender);
	void animationDone(Node* pSender);
	void shockwaveDone();

	void onTouchesBegan(const std::vector<Touch*>& touches, Event *event);

	virtual void update(float dt);

private:
	Vector<Sprite*> * _meteorPool;
	int _meteorPoolIndex;

	Vector<Sprite*> * _healthPool;
	int _healthPoolIndex;

	Vector<Sprite*> * _fallingObjects;
	Vector<Sprite*> * _clouds;

	SpriteBatchNode * _gameBatchNode;
	Sprite * _bomb;
	Sprite * _shockWave;

	Sprite * _introMessage;
	Sprite * _gameOverMessage;

	LabelBMFont * _energyDisplay;
	LabelBMFont * _scoreDisplay;

	Size _screenSize;

	float _meteorInterval;
	float _meteorTimer;
	float _meteorSpeed;
	float _healthInterval;
	float _healthTimer;
	float _healthSpeed;
	float _difficultyInterval;
	float _difficultyTimer;

	int _energy;
	int _score;
	int _shockwaveHits;
	bool _running;

	void resetMeteor(void);
	void resetHealth(void);
	void resetGame(void);
	void stopGame(void);
	void increaseDifficulty(void);

	void createGameScreen(void);
	void createPools(void);
	void createActions(void);
};

#endif // __HELLOWORLD_SCENE_H__
