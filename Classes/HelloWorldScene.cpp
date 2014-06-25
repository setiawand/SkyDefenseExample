#include "HelloWorldScene.h"

USING_NS_CC;

Scene* HelloWorld::createScene() {
	// 'scene' is an autorelease object
	auto scene = Scene::create();

	// 'layer' is an autorelease object
	auto layer = HelloWorld::create();

	// add layer as a child to scene
	scene->addChild(layer);

	// return the scene
	return scene;
}

// on "init" you need to initialize your instance
bool HelloWorld::init() {
	//////////////////////////////
	// 1. super init first
	if (!Layer::init()) {
		return false;
	}

	auto listener = EventListenerTouchAllAtOnce::create();
	listener->onTouchesBegan = CC_CALLBACK_2(HelloWorld::onTouchesBegan, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

	Size visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();
	_screenSize = visibleSize;
	/////////////////////////////
	// 2. add a menu item with "X" image, which is clicked to quit the program
	//    you may modify it.

	// add a "close" icon to exit the progress. it's an autorelease object
	auto closeItem = MenuItemImage::create("CloseNormal.png",
			"CloseSelected.png",
			CC_CALLBACK_1(HelloWorld::menuCloseCallback, this));

	closeItem->setPosition(
			Vec2(
					origin.x + visibleSize.width
							- closeItem->getContentSize().width / 2,
					origin.y + closeItem->getContentSize().height / 2));

	// create menu, it's an autorelease object
	auto menu = Menu::create(closeItem, NULL);
	menu->setPosition(Vec2::ZERO);
	this->addChild(menu, 1);

	_running = false;

	//create game screen elements
	createGameScreen();

	//create object pools
	createPools();

	//create CCActions
	createActions();

	//create array to store all falling objects (will use it in collision check)
	_fallingObjects = new Vector<Sprite *>(40);

	//create main loop
	this->schedule(schedule_selector(HelloWorld::update));

	SimpleAudioEngine::getInstance()->playBackgroundMusic("background.mp3",
			true);

	return true;
}

void HelloWorld::update(float dt) {

	if (!_running)
		return;

	int count;
	int i;

	Sprite * sprite;

	//update timers
	_meteorTimer += dt;
	if (_meteorTimer > _meteorInterval) {
		_meteorTimer = 0;
		this->resetMeteor();
	}

	_healthTimer += dt;
	if (_healthTimer > _healthInterval) {
		_healthTimer = 0;
		this->resetHealth();
	}

	_difficultyTimer += dt;
	if (_difficultyTimer > _difficultyInterval) {
		_difficultyTimer = 0;
		this->increaseDifficulty();
	}

	if (_bomb->isVisible()) {
		if (_bomb->getScale() > 0.3f) {
			if (_bomb->getOpacity() != 255)
				_bomb->setOpacity(255);
		}
	}

	//check collision with shockwave
	if (_shockWave->isVisible()) {
		count = _fallingObjects->size();

		float diffx;
		float diffy;

		for (i = count - 1; i >= 0; i--) {
			sprite = (Sprite *) _fallingObjects->at(i);
			diffx = _shockWave->getPositionX() - sprite->getPositionX();
			diffy = _shockWave->getPositionY() - sprite->getPositionY();
			if (pow(diffx, 2) + pow(diffy, 2)
					<= pow(_shockWave->boundingBox().size.width * 0.5f, 2)) {
				sprite->stopAllActions();

				//sprite animations
				auto animation = Animation::create();
				SpriteFrame * frame;
				//animation for falling object explosion
				for (int i = 1; i <= 7; i++) {
					char szName[100] = { 0 };
					sprintf(szName, "explosion_small%i.png", i);
					frame = SpriteFrameCache::getInstance()->spriteFrameByName(
							szName);
					animation->addSpriteFrame(frame);
				}

				animation->setDelayPerUnit(0.5 / 7.0f);
				animation->setRestoreOriginalFrame(true);
				auto _explosion = Sequence::create(Animate::create(animation),
						CallFuncN::create(this,
								callfuncN_selector(HelloWorld::animationDone)),
						NULL);

				sprite->runAction(_explosion);
				SimpleAudioEngine::getInstance()->playEffect("boom.wav");
				if (sprite->getTag() == kSpriteMeteor) {
					_shockwaveHits++;
					_score += _shockwaveHits * 13 + _shockwaveHits * 2;
				}
				//play sound
				_fallingObjects->erase(i);
			}
		}

		char szValue[100] = { 0 };
		sprintf(szValue, "%i", _score);
		_scoreDisplay->setString(szValue);
	}

	//move clouds
	count = _clouds->size();
	for (i = 0; i < count; i++) {
		sprite = (Sprite *) _clouds->at(i);
		sprite->setPositionX(sprite->getPositionX() + dt * 20);
		if (sprite->getPositionX()
				> _screenSize.width + sprite->boundingBox().size.width * 0.5f)
			sprite->setPositionX(-sprite->boundingBox().size.width * 0.5f);
	}

}

void HelloWorld::onTouchesBegan(const std::vector<Touch*>& touches,
		Event *event) {
	CCLog("onTouchBegan");
	//if game not running, we are seeing either intro or gameover
	if (!_running) {
		//if intro, hide intro message
		if (_introMessage->isVisible()) {
			_introMessage->setVisible(false);

			//if game over, hide game over message
		} else if (_gameOverMessage->isVisible()) {
			SimpleAudioEngine::getInstance()->stopAllEffects();
			_gameOverMessage->setVisible(false);

		}

		this->resetGame();
		return;
	}

	for (auto &item : touches) {
		Touch *touch = item;
		if (touch) {
			_bomb->stopAllActions();
			if (_bomb->isVisible()) {
				Sprite * child;
				child = (Sprite *) _bomb->getChildByTag(kSpriteHalo);
				child->stopAllActions();
				child = (Sprite *) _bomb->getChildByTag(kSpriteSparkle);
				child->stopAllActions();
				//if bomb is the right size, then create shockwave
				if (_bomb->getScale() > 0.3f) {
					_shockWave->setScale(0.1f);
					_shockWave->setPosition(_bomb->getPosition());
					_shockWave->setVisible(true);
					_shockWave->runAction(
							ScaleTo::create(0.5f, _bomb->getScale() * 2.0f));
					//action sequence for shockwave: fade out, call back when done
					auto _shockwaveSequence =
							Sequence::create(FadeOut::create(1.0f),
									CallFunc::create(this,
											callfunc_selector(HelloWorld::shockwaveDone)),
									NULL);
					_shockWave->runAction(_shockwaveSequence);
					SimpleAudioEngine::getInstance()->playEffect(
							"bombRelease.wav");
				} else {
					SimpleAudioEngine::getInstance()->playEffect(
							"bombFail.wav");
				}
				_bomb->setVisible(false);
				//reset hits with shockwave, so we can count combo hits
				_shockwaveHits = 0;

				//if no bomb currently on screen, create one
			} else {
				Vec2 tap = touch->getLocation();
				_bomb->setScale(0.1f);
				_bomb->setPosition(tap);
				_bomb->setVisible(true);
				_bomb->setOpacity(50);
				//action to grow bomb
				_bomb->runAction(ScaleTo::create(6.0f, 1));
				Sprite * child;
				child = (Sprite *) _bomb->getChildByTag(kSpriteHalo);
				//action to rotate sprites
				auto _rotateSprite = RepeatForever::create(
						RotateBy::create(0.5f, -90));
				child->runAction(_rotateSprite);
				child = (Sprite *) _bomb->getChildByTag(kSpriteSparkle);
				child->runAction(_rotateSprite);
			}
		}
	}
}

//call back for when falling object reaches its target
void HelloWorld::fallingObjectDone(Node* pSender) {

	//remove it from array
	_fallingObjects->eraseObject((Sprite*) pSender);
	pSender->stopAllActions();
	pSender->setRotation(0);

	//if object is a meteor...
	if (pSender->getTag() == kSpriteMeteor) {
		_energy -= 15;
		//show explosion animation
		//animation for ground hit
		auto animation = Animation::create();
		SpriteFrame * frame;
		for (int i = 1; i <= 10; i++) {
			char szName[100] = { 0 };
			sprintf(szName, "boom%i.png", i);
			frame = SpriteFrameCache::getInstance()->spriteFrameByName(szName);
			animation->addSpriteFrame(frame);
		}

		animation->setDelayPerUnit(1 / 10.0f);
		animation->setRestoreOriginalFrame(true);
		auto _groundHit = Sequence::create(
				MoveBy::create(0, Vec2(0, _screenSize.height * 0.12f)),
				Animate::create(animation),
				CallFuncN::create(this,
						callfuncN_selector(HelloWorld::animationDone)), NULL);
		pSender->runAction(_groundHit);
		//play explosion sound
		SimpleAudioEngine::getInstance()->playEffect("boom.wav");

		//if object is a health drop...
	} else {
		pSender->setVisible(false);

		//if energy is full, score points from falling drop
		if (_energy == 100) {

			_score += 25;
			char score[100] = { 0 };
			sprintf(score, "%i", _score);
			_scoreDisplay->setString(score);

		} else {
			_energy += 10;
			if (_energy > 100)
				_energy = 100;
		}

		//play health bonus sound
		SimpleAudioEngine::getInstance()->playEffect("health.wav");
	}

	//if energy is less or equal 0, game over
	if (_energy <= 0) {
		_energy = 0;
		this->stopGame();
		SimpleAudioEngine::getInstance()->playEffect("fire_truck.wav");
		//show GameOver
		_gameOverMessage->setVisible(true);
	}

	char szValue[100] = { 0 };
	sprintf(szValue, "%i%s", _energy, "%");
	_energyDisplay->setString(szValue);

}

//call back for animation done (hide animated object)
void HelloWorld::animationDone(CCNode* pSender) {
	pSender->setVisible(false);
}

//call back for shockwave done, hide shockwave
void HelloWorld::shockwaveDone() {
	_shockWave->setVisible(false);
}

//use a new meteor from the pool
void HelloWorld::resetMeteor(void) {

	//if too many objects on screen, return
	if (_fallingObjects->size() > 30)
		return;

	auto meteor = _meteorPool->at(_meteorPoolIndex);
	_meteorPoolIndex++;
	if (_meteorPoolIndex == _meteorPool->size())
		_meteorPoolIndex = 0;

	//pick start and target positions for new meteor
	int meteor_x = rand() % (int) (_screenSize.width * 0.8f)
			+ _screenSize.width * 0.1f;
	int meteor_target_x = rand() % (int) (_screenSize.width * 0.8f)
			+ _screenSize.width * 0.1f;

	meteor->stopAllActions();
	meteor->setPosition(
			Vec2(meteor_x,
					_screenSize.height
							+ meteor->boundingBox().size.height * 0.5));

	//create action for meteor (rotate forever, move to target, and call function)
	auto sequence = Sequence::create(
			MoveTo::create(_meteorSpeed,
					Vec2(meteor_target_x, _screenSize.height * 0.15f)),
			CallFuncN::create(this,
					callfuncN_selector(HelloWorld::fallingObjectDone)), NULL);

	meteor->setVisible(true);
	meteor->runAction(RepeatForever::create(RotateBy::create(0.5f, -90)));
	meteor->runAction(sequence);
	_fallingObjects->pushBack(meteor);

}

//use a new health drop from the pool
void HelloWorld::resetHealth(void) {

	if (_fallingObjects->size() > 30)
		return;

	auto health = _healthPool->at(_healthPoolIndex);
	_healthPoolIndex++;
	if (_healthPoolIndex == _healthPool->size())
		_healthPoolIndex = 0;

	int health_x = rand() % (int) (_screenSize.width * 0.8f)
			+ _screenSize.width * 0.1f;
	int health_target_x = rand() % (int) (_screenSize.width * 0.8f)
			+ _screenSize.width * 0.1f;

	health->stopAllActions();
	health->setPosition(
			Vec2(health_x,
					_screenSize.height
							+ health->boundingBox().size.height * 0.5));

	//create action (swing, move to target, and call function when done)
	FiniteTimeAction* sequence = Sequence::create(
			MoveTo::create(_healthSpeed,
					Vec2(health_target_x, _screenSize.height * 0.15f)),
			CallFuncN::create(this,
					callfuncN_selector(HelloWorld::fallingObjectDone)), NULL);

	health->setVisible(true);

	//swing action for health drops
	FiniteTimeAction* easeSwing = Sequence::create(
			EaseInOut::create(RotateTo::create(1.2f, -10), 2),
			EaseInOut::create(RotateTo::create(1.2f, 10), 2), NULL);
	auto _swingHealth = RepeatForever::create((ActionInterval*) easeSwing);
	health->runAction(_swingHealth);
	health->runAction(sequence);
	_fallingObjects->pushBack(health);
}

//start
void HelloWorld::resetGame(void) {

	_score = 0;
	_energy = 100;

	//reset timers and "speeds"
	_meteorInterval = 2.5;
	_meteorTimer = _meteorInterval * 0.99f;
	_meteorSpeed = 10; //in seconds to reach ground
	_healthInterval = 20;
	_healthTimer = 0;
	_healthSpeed = 15; //in seconds to reach ground

	_difficultyInterval = 60;
	_difficultyTimer = 0;

	_running = true;

	//reset labels
	char energy[100] = { 0 };
	sprintf(energy, "%i%s", _energy, "%");
	_energyDisplay->setString(energy);

	char score[100] = { 0 };
	sprintf(score, "%i", _score);
	_scoreDisplay->setString(score);

}

//this function is called at regular intervals
void HelloWorld::increaseDifficulty() {

	_meteorInterval -= 0.2f;
	if (_meteorInterval < 0.25f)
		_meteorInterval = 0.25f;
	_meteorSpeed -= 1;
	if (_meteorSpeed < 4)
		_meteorSpeed = 4;
	_healthSpeed -= 1;
	if (_healthSpeed < 8)
		_healthSpeed = 8;

}

//if player is killed, stop all actions, clear screen
void HelloWorld::stopGame() {

	_running = false;
	//stop all actions currently running (meteors, health drops, animations...)
	int i;
	int count;
	Sprite * sprite;
	count = _fallingObjects->size();
	for (i = count - 1; i >= 0; i--) {
		sprite = _fallingObjects->at(i);
		sprite->stopAllActions();
		sprite->setVisible(false);
		_fallingObjects->erase(i);
	}
	if (_bomb->isVisible()) {
		_bomb->stopAllActions();
		_bomb->setVisible(false);
		Sprite * child;
		child = (Sprite *) _bomb->getChildByTag(kSpriteHalo);
		child->stopAllActions();
		child = (Sprite *) _bomb->getChildByTag(kSpriteSparkle);
		child->stopAllActions();
	}
	if (_shockWave->isVisible()) {
		_shockWave->stopAllActions();
		_shockWave->setVisible(false);
	}

}

void HelloWorld::createGameScreen() {

	//add bg
	auto bg = Sprite::create("bg.png");
	bg->setPosition(Vec2(_screenSize.width * 0.5f, _screenSize.height * 0.5f));
	this->addChild(bg);

	//create spritebatch node
	SpriteFrameCache::sharedSpriteFrameCache()->addSpriteFramesWithFile(
			"sprite_sheet.plist");
	_gameBatchNode = SpriteBatchNode::create("sprite_sheet.png");
	this->addChild(_gameBatchNode);

	//create cityscape
	Sprite * sprite;
	for (int i = 0; i < 2; i++) {
		sprite = Sprite::createWithSpriteFrameName("city_dark.png");
		sprite->setPosition(
				Vec2(_screenSize.width * (0.25f + i * 0.5f),
						sprite->boundingBox().size.height * 0.5f));
		_gameBatchNode->addChild(sprite, kForeground);

		sprite = Sprite::createWithSpriteFrameName("city_light.png");
		sprite->setPosition(
				Vec2(_screenSize.width * (0.25f + i * 0.5f),
						sprite->boundingBox().size.height * 0.9f));
		_gameBatchNode->addChild(sprite, kBackground);
	}

	//add trees
	for (int i = 0; i < 3; i++) {
		sprite = Sprite::createWithSpriteFrameName("trees.png");
		sprite->setPosition(
				Vec2(_screenSize.width * (0.2f + i * 0.3f),
						sprite->boundingBox().size.height * 0.5f));
		_gameBatchNode->addChild(sprite, kForeground);

	}

	//add HUD
	_scoreDisplay = LabelBMFont::create("0", "font.fnt",
			_screenSize.width * 0.3f);
	_scoreDisplay->setAnchorPoint(Vec2(1, 0.5));
	_scoreDisplay->setPosition(
			Vec2(_screenSize.width * 0.8f, _screenSize.height * 0.94f));
	this->addChild(_scoreDisplay);

	_energyDisplay = LabelBMFont::create("100%", "font.fnt",
			_screenSize.width * 0.1f, kCCTextAlignmentRight);
	_energyDisplay->setPosition(
			Vec2(_screenSize.width * 0.3f, _screenSize.height * 0.94f));
	this->addChild(_energyDisplay);

	Sprite * icon = Sprite::createWithSpriteFrameName("health_icon.png");
	icon->setPosition(
			Vec2(_screenSize.width * 0.15f, _screenSize.height * 0.94f));
	_gameBatchNode->addChild(icon, kBackground);

	//add clouds
	Sprite * cloud;
	_clouds = new Vector<Sprite*>(4);
	float cloud_y;
	for (int i = 0; i < 4; i++) {
		cloud_y =
				i % 2 == 0 ?
						_screenSize.height * 0.4f : _screenSize.height * 0.5f;
		cloud = Sprite::createWithSpriteFrameName("cloud.png");
		cloud->setPosition(
				Vec2(_screenSize.width * 0.1f + i * _screenSize.width * 0.3f,
						cloud_y));
		_gameBatchNode->addChild(cloud, kBackground);
		_clouds->pushBack(cloud);
	}

	//CREATE BOMB SPRITE
	_bomb = Sprite::createWithSpriteFrameName("bomb.png");
	_bomb->getTexture()->generateMipmap();
	_bomb->setVisible(false);

	Size size = _bomb->boundingBox().size;

	//add sparkle inside bomb sprite
	Sprite * sparkle = Sprite::createWithSpriteFrameName("sparkle.png");
	sparkle->setPosition(Vec2(size.width * 0.72f, size.height * 0.72f));
	_bomb->addChild(sparkle, kMiddleground, kSpriteSparkle);

	//add halo inside bomb sprite
	CCSprite * halo = Sprite::createWithSpriteFrameName("halo.png");
	halo->setPosition(Vec2(size.width * 0.4f, size.height * 0.4f));
	_bomb->addChild(halo, kMiddleground, kSpriteHalo);
	_gameBatchNode->addChild(_bomb, kForeground);

	//add shockwave
	_shockWave = Sprite::createWithSpriteFrameName("shockwave.png");
	_shockWave->getTexture()->generateMipmap();
	_shockWave->setVisible(false);
	_gameBatchNode->addChild(_shockWave);

	//intro message
	_introMessage = Sprite::createWithSpriteFrameName("logo.png");
	_introMessage->setPosition(
			Vec2(_screenSize.width * 0.5f, _screenSize.height * 0.6f));
	_introMessage->setVisible(true);
	this->addChild(_introMessage, kForeground);

	//game over message
	_gameOverMessage = Sprite::createWithSpriteFrameName("gameover.png");
	_gameOverMessage->setPosition(
			Vec2(_screenSize.width * 0.5f, _screenSize.height * 0.65f));
	_gameOverMessage->setVisible(false);
	this->addChild(_gameOverMessage, kForeground);

}

void HelloWorld::createPools() {

	Sprite * sprite;
	int i;

	//create meteor pool
	_meteorPool = new Vector<Sprite*>(50);
	_meteorPoolIndex = 0;
	for (i = 0; i < 50; i++) {
		sprite = Sprite::createWithSpriteFrameName("meteor.png");
		sprite->setVisible(false);
		_gameBatchNode->addChild(sprite, kMiddleground, kSpriteMeteor);
		_meteorPool->pushBack(sprite);
	}

	//create health pool
	_healthPool = new Vector<Sprite*>(20);
	_healthPoolIndex = 0;
	for (i = 0; i < 20; i++) {
		sprite = Sprite::createWithSpriteFrameName("health.png");
		sprite->setVisible(false);
		sprite->setAnchorPoint(Vec2(0.5f, 0.8f));
		_gameBatchNode->addChild(sprite, kMiddleground, kSpriteHealth);
		_healthPool->pushBack(sprite);
	}

}

void HelloWorld::createActions() {

}

void HelloWorld::menuCloseCallback(Ref* pSender) {
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WP8) || (CC_TARGET_PLATFORM == CC_PLATFORM_WINRT)
	MessageBox("You pressed the close button. Windows Store Apps do not implement a close button.","Alert");
	return;
#endif

	Director::getInstance()->end();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
	exit(0);
#endif
}
