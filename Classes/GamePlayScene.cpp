#include "GamePlayScene.h"
#include "SimpleAudioEngine.h"

#define PTM_RATIO 32

enum {
	kTagTileMap = 1, kTagBatchNode = 1, kTagAnimation1 = 1,
};

using namespace cocos2d;
using namespace CocosDenshion;

b2World* world;
GLESDebugDraw *m_debugDraw;

b2RevoluteJoint *motor1;
b2RevoluteJoint *motor2;
b2Body *cart;
b2PrismaticJoint *spring1;
b2PrismaticJoint *spring2;
float currentTick;

int speed;

CCScene* GamePlayScene::scene() {
	// 'scene' is an autorelease object
	CCScene *scene = CCScene::create();

	// 'layer' is an autorelease object
	GamePlayScene *layer = GamePlayScene::create();

	// add layer as a child to scene
	scene->addChild(layer);

	// return the scene
	return scene;
}

// on "init" you need to initialize your instance
bool GamePlayScene::init() {
	//////////////////////////////
	// 1. super init first
	if (!CCLayer::init()) {
		return false;
	}

	CCSize winSize = CCDirector::sharedDirector()->getWinSize();

	b2Vec2 gravity;
	gravity.Set(0.0f, -10.0f);

	world = new b2World(gravity);

	world->SetContinuousPhysics(true);
	world->SetAllowSleeping(true);

	m_debugDraw = new GLESDebugDraw(PTM_RATIO);
	world->SetDebugDraw(m_debugDraw);

	uint32 flags = 0;
	flags += b2Draw::e_shapeBit;

	m_debugDraw->SetFlags(flags);

	// Define the ground body.
	b2BodyDef groundBodyDef;
	groundBodyDef.position.Set(0, 0); // bottom-left corner
	b2Body* groundBody = world->CreateBody(&groundBodyDef);

	// Define the ground box shape.
	b2EdgeShape groundBox;

	b2FixtureDef fixtureDef;

	fixtureDef.shape = &groundBox;
	fixtureDef.density = 1.0f;
	fixtureDef.friction = 0.5f;
	fixtureDef.restitution = 0.2f;

	// bottom
	groundBox.Set(b2Vec2(0, 0), b2Vec2(4000 / PTM_RATIO, 0));
	groundBody->CreateFixture(&fixtureDef);

	// top
	groundBox.Set(b2Vec2(0, winSize.height / PTM_RATIO),
			b2Vec2(winSize.width / PTM_RATIO, winSize.height / PTM_RATIO));
	groundBody->CreateFixture(&groundBox, 0);

	// left
	groundBox.Set(b2Vec2(0, winSize.height / PTM_RATIO), b2Vec2(0, 0));
	groundBody->CreateFixture(&groundBox, 0);

	// right
	groundBox.Set(b2Vec2(4000 / PTM_RATIO, winSize.height / PTM_RATIO),
			b2Vec2(4000 / PTM_RATIO, 0));
	groundBody->CreateFixture(&groundBox, 0);

	//scenario
	b2BodyDef obs1BodyDef;
	obs1BodyDef.position.Set(1500 / PTM_RATIO, 30 / PTM_RATIO);
	b2Body* obs1Body = world->CreateBody(&obs1BodyDef);
	b2PolygonShape obs1ShapeBox;
	obs1ShapeBox.SetAsBox(120 / PTM_RATIO, 40 / PTM_RATIO, b2Vec2(0, 0),
			M_PI / 6);

	b2FixtureDef obs1FixtureDef;

	obs1FixtureDef.shape = &obs1ShapeBox;
	obs1FixtureDef.density = 1.0f;
	obs1FixtureDef.friction = 0.5f;
	obs1FixtureDef.restitution = 0.2f;

	obs1Body->CreateFixture(&obs1FixtureDef);

	b2BodyDef obs2BodyDef;
	obs2BodyDef.position.Set(1800 / PTM_RATIO, 30 / PTM_RATIO);
	b2Body* obs2Body = world->CreateBody(&obs2BodyDef);
	b2PolygonShape obs2ShapeBox;
	obs2ShapeBox.SetAsBox(120 / PTM_RATIO, 40 / PTM_RATIO, b2Vec2(0, 0),
			-M_PI / 6);

	b2FixtureDef obs2FixtureDef;

	obs2FixtureDef.shape = &obs2ShapeBox;
	obs2FixtureDef.density = 1.0f;
	obs2FixtureDef.friction = 0.5f;
	obs2FixtureDef.restitution = 0.2f;

	obs2Body->CreateFixture(&obs2FixtureDef);

	CCLabelTTF *label = CCLabelTTF::create("Tap screen", "Mark Felt", 32);
	this->addChild(label);
	label->setColor(ccc3(0, 0, 255));
	label->setPosition(ccp(winSize.width / 2, winSize.height - 50));

	this->addCart();

	this->schedule(schedule_selector(GamePlayScene::tick));

	CCDirector::sharedDirector()->getTouchDispatcher()->addTargetedDelegate(
			this, 0, true);

	this->setTouchEnabled(true);

	return true;
}

void GamePlayScene::draw() {
	glDisable( GL_TEXTURE_2D);
	world->DrawDebugData();
	glEnable(GL_TEXTURE_2D);
}

void GamePlayScene::addCart() {

	CCLOG("Add new cart");

	CCSprite *cartBaseSprt = CCSprite::create("cartBase.png");

	this->addChild(cartBaseSprt);

	cartBaseSprt->setPosition(ccp(100, 80));

	CCPoint carStartPosition = ccp(100, 75);

	b2BodyDef comonCartbodyDef;
	comonCartbodyDef.position.Set(carStartPosition.x / PTM_RATIO,
			carStartPosition.y / PTM_RATIO);
	comonCartbodyDef.userData = cartBaseSprt;
	comonCartbodyDef.type = b2_dynamicBody;

	cart = world->CreateBody(&comonCartbodyDef);

	b2PolygonShape comonCartPolygonShape;
	comonCartPolygonShape.SetAsBox(1.5f, 0.3f);

	b2FixtureDef cartBaseFixture;

	cartBaseFixture.shape = &comonCartPolygonShape;
	cartBaseFixture.density = 2.0f;
	cartBaseFixture.friction = 1.0f;
	cartBaseFixture.restitution = 0.2f;
	cartBaseFixture.filter.groupIndex = -1;

	cart->CreateFixture(&cartBaseFixture);

	comonCartPolygonShape.SetAsBox(0.4, 0.15, b2Vec2(-1, -0.3), M_PI / 3);

	b2FixtureDef axle1fixtureDef;

	axle1fixtureDef.shape = &comonCartPolygonShape;
	axle1fixtureDef.density = 2.0f;
	axle1fixtureDef.friction = 0.0f;
	axle1fixtureDef.restitution = 0.2f;
	axle1fixtureDef.filter.groupIndex = -1;

	cart->CreateFixture(&axle1fixtureDef);

	comonCartPolygonShape.SetAsBox(0.4, 0.15, b2Vec2(1, -0.3), -M_PI / 3);


	b2FixtureDef axle2fixtureDef;

	axle2fixtureDef.shape = &comonCartPolygonShape;
	axle2fixtureDef.density = 2.0f;
	axle2fixtureDef.friction = 0.0f;
	axle2fixtureDef.restitution = 0.2f;
	axle2fixtureDef.filter.groupIndex = -1;

	cart->CreateFixture(&axle2fixtureDef);


	//axles

	b2Body *axle1Body;
	axle1Body = world->CreateBody(&comonCartbodyDef);

	comonCartPolygonShape.SetAsBox(0.4, 0.1,
			b2Vec2(-1 - 0.6 * cos(M_PI / 3), -0.3 - 0.6 * sin(M_PI / 3)),
			M_PI / 3);
	axle1Body->CreateFixture(&comonCartPolygonShape, 1);

	b2PrismaticJointDef prismaticJointDef;

	prismaticJointDef.Initialize(cart, axle1Body, axle1Body->GetWorldCenter(),
			b2Vec2(cos(M_PI / 3), sin(M_PI / 3)));
	prismaticJointDef.lowerTranslation = -0.3;
	prismaticJointDef.upperTranslation = 0.5;
	prismaticJointDef.enableLimit = true;
	prismaticJointDef.enableMotor = true;
	prismaticJointDef.collideConnected = false;

	spring1 = (b2PrismaticJoint*) world->CreateJoint(&prismaticJointDef);

	b2Body *axle2Body = world->CreateBody(&comonCartbodyDef);

	comonCartPolygonShape.SetAsBox(0.4, 0.1,
			b2Vec2(1 + 0.6 * cos(-M_PI / 3), -0.3 + 0.6 * sin(-M_PI / 3)),
			-M_PI / 3);
	axle2Body->CreateFixture(&comonCartPolygonShape, 1);

	prismaticJointDef.Initialize(cart, axle2Body, axle2Body->GetWorldCenter(),
			b2Vec2(-cos(M_PI / 3), sin(M_PI / 3)));

	spring2 = (b2PrismaticJoint*) world->CreateJoint(&prismaticJointDef);

	b2CircleShape circleDef;
	circleDef.m_radius = 0.7f;

	b2FixtureDef circleFixtureDef;

	// add wheels //
	circleFixtureDef.shape = &circleDef;
	circleFixtureDef.density = 1.0f;
	circleFixtureDef.friction = 1.0f;
	circleFixtureDef.restitution = 0.2;
	circleFixtureDef.filter.groupIndex = -1;

	b2Body *bodyWheel1;
	b2Body *bodyWheel2;

	for (int i = 0; i < 2; i++) {

		b2BodyDef circleBodyDef;
		circleBodyDef.type = b2_dynamicBody;

		if (i == 0)
			circleBodyDef.position.Set(
					axle1Body->GetWorldCenter().x - 0.3 * cos(M_PI / 3),
					axle1Body->GetWorldCenter().y - 0.3 * sin(M_PI / 3));
		else
			circleBodyDef.position.Set(
					axle2Body->GetWorldCenter().x + 0.3 * cos(-M_PI / 3),
					axle2Body->GetWorldCenter().y + 0.3 * sin(-M_PI / 3));
		circleBodyDef.allowSleep = false;

		if (i == 0) {
			bodyWheel1 = world->CreateBody(&circleBodyDef);
			bodyWheel1->CreateFixture(&circleFixtureDef);
		} else {
			bodyWheel2 = world->CreateBody(&circleBodyDef);
			bodyWheel2->CreateFixture(&circleFixtureDef);
		}
	}

	// add joints //
	b2RevoluteJointDef motorRevJointDef;

	motorRevJointDef.enableMotor = true;
	motorRevJointDef.collideConnected = false;

	motorRevJointDef.Initialize(axle1Body, bodyWheel1, bodyWheel1->GetWorldCenter());
	motor1 = (b2RevoluteJoint *) world->CreateJoint(&motorRevJointDef);

	motorRevJointDef.Initialize(axle2Body, bodyWheel2, bodyWheel2->GetWorldCenter());
	motor2 = (b2RevoluteJoint *) world->CreateJoint(&motorRevJointDef);
}

void GamePlayScene::tick(float dt) {

	currentTick = dt;

	int32 velocityIterations = 8;
	int32 positionIterations = 1;

	world->Step(dt, velocityIterations, positionIterations);

	motor2->SetMotorSpeed(-15 * M_PI * speed);

	if (abs(speed) == 1) {
		motor2->SetMaxMotorTorque(17.0f);
	} else {
		motor2->SetMaxMotorTorque(0.5f);
	}

	motor1->SetMotorSpeed(-15 * M_PI * speed);

	if (abs(speed) == 1) {
		motor1->SetMaxMotorTorque(12.0f);
	} else {
		motor1->SetMaxMotorTorque(0.5f);
	}

	spring2->SetMaxMotorForce(
			20 + fabsf(800 * pow(spring2->GetJointTranslation(), 2)));
	spring2->SetMotorSpeed(-4 * pow(spring2->GetJointTranslation(), 1));

	spring1->SetMaxMotorForce(
			30 + fabsf(800 * pow(spring1->GetJointTranslation(), 2)));
	spring1->SetMotorSpeed(
			(spring1->GetMotorSpeed() - 10 * spring1->GetJointTranslation())
					* 0.4);

	b2Vec2 reactF1 = spring1->GetReactionForce(dt);
	b2Vec2 reactF2 = spring2->GetReactionForce(dt);

	float32 rt1 = spring1->GetReactionTorque(dt);
	float32 rt2 = spring2->GetReactionTorque(dt);

	if ((fabsf(rt2) > 0.2) || (fabsf(rt1) > 0.2)) {
		CCLOG("Explode the joints rt1 %f and rt2 %f!!!", rt1, rt2);
	}

	if ((fabsf(reactF1.x) > 0.2) || (fabsf(reactF1.y)) > 0.2) {
		CCLOG("Explode the joints F1x %f and F1y %f!!!", reactF1.x, reactF1.y);
	}

	if ((fabsf(reactF2.x) > 0.2) || (fabsf(reactF2.y)) > 0.2) {
		CCLOG("Explode the joints F2x %f and F2y %f!!!", reactF2.x, reactF2.y);
	}

	// Instruct the world to perform a single step of simulation. It is
	// generally best to keep the time step and iterations fixed.

	//Iterate over the bodies in the physics world
	for (b2Body* b = world->GetBodyList(); b; b = b->GetNext()) {
		if (b->GetUserData() != NULL) {
			//Synchronize the AtlasSprites position and rotation with the corresponding body
			CCSprite *myActor = (CCSprite*) b->GetUserData();
			myActor->setPosition(
					ccp(b->GetPosition().x * PTM_RATIO,
							b->GetPosition().y * PTM_RATIO));
			myActor->setRotation(-1 * CC_RADIANS_TO_DEGREES(b->GetAngle()));
		}
	}

	b2Vec2 pos = cart->GetPosition();
	CCPoint newPos = ccp(-1 * pos.x * PTM_RATIO + 150,
			this->getPosition().y * PTM_RATIO);
	this->setPosition(newPos);
}

bool GamePlayScene::ccTouchBegan(CCTouch* touch, CCEvent* event) {
	CCLOG("Touch Began");
	CCPoint location = this->convertTouchToNodeSpace(touch);
	b2Vec2 cartPosition = cart->GetPosition();
	if (location.x > cartPosition.x * PTM_RATIO) {
		speed = 1;
	} else {
		speed = -1;
	}
}

void GamePlayScene::ccTouchMoved(CCTouch* touch, CCEvent* event) {

}

void GamePlayScene::ccTouchEnded(CCTouch* touch, CCEvent* event) {
	CCLOG("Touch Ended");
	speed = 0;
}


void GamePlayScene::menuCloseCallback(CCObject* pSender) {
	CCDirector::sharedDirector()->end();
	#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
	exit(0);
	#endif
}
