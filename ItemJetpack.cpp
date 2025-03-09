#include "ItemJetpack.h"

MeshRenderer ItemJetpack::renderer{};
std::string ItemJetpack::flightSound = "";

int ItemJetpack::getSelectedFuelCount(InventoryPlayer& inventory) {
	int count = 0;
	for (int slot = 0;slot < inventory.getSlotCount(); slot++) {
		Item* i = inventory.getSlot(slot)->get();
		if (i != nullptr && i->getName() == (isSelectedFuelDeadly? "Deadly Fuel" : "Biofuel"))
			count += i->count;

	}
	return count;
}

void ItemJetpack::consumeSelectedFuel(InventoryPlayer& inventory) {
	for (int slot = 0;slot < inventory.getSlotCount(); slot++) {
		Item* i = inventory.getSlot(slot)->get();
		if (i != nullptr && i->getName() == (isSelectedFuelDeadly ? "Deadly Fuel" : "Biofuel")) {
			i->count--;
			if (i->count < 1) inventory.getSlot(slot)->reset();
			fuelLevel = 1.0f;
			isFuelDeadly = isSelectedFuelDeadly;
			return;
		}
	}
}


stl::string ItemJetpack::getName() {
	return "Jetpack";
}

void ItemJetpack::handleFlight(Player* player) {
	static double lastTime = glfwGetTime() - 0.01;
	double curTime = glfwGetTime();
	double dt = curTime - lastTime;
	lastTime = curTime;

	if (fuelLevel <= 0.0f && player->touchingGround) consumeSelectedFuel(player->inventoryAndEquipment);
	if (player->touchingGround) return;
	if (fuelLevel <= 0.0f) return;

	float thrust = isFuelDeadly ? 100 : 70;

	if (flightMode==Flight) {
		if (!player->keys.space) return;
		player->vel.y=std::min(player->vel.y + thrust*(float)dt , isFuelDeadly ? 15.0f : 12.0f);
		fuelLevel -= (float)dt * (isFuelDeadly ? 0.05 : 0.15);
		AudioManager::playSound4D(flightSound, "ambience", player->cameraPos, { 0,0,0,0 });
	}
	else if (flightMode == Hover){
		if (player->keys.space) {
			player->vel.y = std::min(player->vel.y + thrust * (float)dt, isFuelDeadly ? 10.0f : 8.0f);
			fuelLevel -= (float)dt * (isFuelDeadly ? 0.04 : 0.13);
		}
		else if (player->keys.shift) {
			player->vel.y = std::max(player->vel.y - thrust, isFuelDeadly ? -7.0f : -5.0f);
			fuelLevel -= (float)dt * (isFuelDeadly ? 0.015 : 0.07);
		}
		else {
			player->vel.y = std::max(player->vel.y - thrust, 0.0f);
			fuelLevel -= (float)dt * (isFuelDeadly ? 0.01 : 0.05);
		}
	}
}

void ItemJetpack::render(const glm::ivec2& pos) {
	TexRenderer& tr = *ItemTool::tr; // or TexRenderer& tr = ItemTool::tr; after 0.3
	const Tex2D* ogTex = tr.texture; // remember the original texture

	tr.texture = ResourceManager::get("assets/Tools.png", true); // set to custom texture
	tr.setClip(0, 0, 36, 36);
	tr.setPos(pos.x, pos.y, 70, 72);
	tr.render();

	tr.texture = ogTex; // return to the original texture

}

void ItemJetpack::renderEntity(const m4::Mat5& MV, bool inHand, const glm::vec4& lightDir) {

	glm::vec3 colorIron{ 181.0f / 255.0f, 179.0f / 255.0f, 174.0f / 255.0f };

	m4::Mat5 controllerHandle = MV;
	controllerHandle.translate(glm::vec4{ 0.0f, 0.0f, 0.0f, 0.001f });
	controllerHandle *= m4::Rotor
	(
		{
			m4::wedge({0, 0, 1, 0}, {0, 1, 0, 0}), // ZY
			-glm::pi<float>() / 6
		}
	);
	controllerHandle.scale(glm::vec4{ 0.2f,1.f,0.25f,0.25f });
	controllerHandle.translate(glm::vec4{ -0.5f, -0.5f, -0.5f, -0.5f });

	m4::Mat5 controllerBase = MV;
	controllerBase.translate(glm::vec4{ 0.0f, .6f, -0.5f, 0.001f });
	controllerBase *= m4::Rotor
	(
		{
			m4::wedge({0, 0, 1, 0}, {0, 1, 0, 0}), // ZY
			-glm::pi<float>() / 3
		}
	);
	controllerBase.scale(glm::vec4{ 0.5f,.7f,0.25f,0.25f });
	controllerBase.translate(glm::vec4{ -0.5f, -0.5f, -0.5f, -0.5f });

	m4::Mat5 controllerLed = MV;
	controllerLed.translate(glm::vec4{ 0.0f, .6f, -0.5f, 0.001f });
	controllerLed *= m4::Rotor
	(
		{
			m4::wedge({0, 0, 1, 0}, {0, 1, 0, 0}), // ZY
			-glm::pi<float>() / 3
		}
	);
	controllerLed.translate(glm::vec4{ 0.0f, 0.0f, 0.125f, 0.0f });
	controllerLed.scale(glm::vec4{ 0.02f,.02f,.02f,.02f });
	controllerLed.translate(glm::vec4{ -0.5f, -0.5f, -0.5f, -0.5f });

	m4::Mat5 controllerButton1 = MV;
	controllerButton1.translate(glm::vec4{ 0.0f, .6f, -0.5f, 0.001f });
	controllerButton1 *= m4::Rotor
	(
		{
			m4::wedge({0, 0, 1, 0}, {0, 1, 0, 0}), // ZY
			-glm::pi<float>() / 3
		}
	);
	controllerLed.translate(glm::vec4{ 0.0f, 0.0f, 0.125f, 0.0f });
	controllerButton1.scale(glm::vec4{ 0.2f,.2f,0.2f,0.2f });
	controllerButton1.translate(glm::vec4{ -0.5f, -0.5f, -0.5f, -0.5f });

	const Shader* shader = ShaderManager::get("tetSolidColorNormalShader");

	shader->use();

	glUniform4f(glGetUniformLocation(shader->id(), "lightDir"), lightDir.x, lightDir.y, lightDir.z, lightDir.w);
	//IRON COLOR
	glUniform4f(glGetUniformLocation(shader->id(), "inColor"), colorIron.r, colorIron.g, colorIron.b, 1);


	glUniform1fv(glGetUniformLocation(shader->id(), "MV"), sizeof(controllerHandle) / sizeof(float), &controllerHandle[0][0]);
	renderer.render();
	glUniform1fv(glGetUniformLocation(shader->id(), "MV"), sizeof(controllerBase) / sizeof(float), &controllerBase[0][0]);
	renderer.render();

	// RED COLOR
	glUniform4f(glGetUniformLocation(shader->id(), "inColor"), 1, 0, 0, 1);

	glUniform1fv(glGetUniformLocation(shader->id(), "MV"), sizeof(controllerLed) / sizeof(float), &controllerLed[0][0]);
	renderer.render();
	
	// SOLENOID COLOR
	glUniform4f(glGetUniformLocation(shader->id(), "inColor"), 69.0f / 255.f, 144.0f / 255.f, 127.0f / 255.f, 1);

	glUniform1fv(glGetUniformLocation(shader->id(), "MV"), sizeof(controllerButton1) / sizeof(float), &controllerButton1[0][0]);
	renderer.render();
}

void ItemJetpack::rendererInit() {
	MeshBuilder mesh{ BlockInfo::HYPERCUBE_FULL_INDEX_COUNT };
	// vertex position attribute
	mesh.addBuff(BlockInfo::hypercube_full_verts, sizeof(BlockInfo::hypercube_full_verts));
	mesh.addAttr(GL_UNSIGNED_BYTE, 4, sizeof(glm::u8vec4));
	// per-cell normal attribute
	mesh.addBuff(BlockInfo::hypercube_full_normals, sizeof(BlockInfo::hypercube_full_normals));
	mesh.addAttr(GL_FLOAT, 1, sizeof(GLfloat));

	mesh.setIndexBuff(BlockInfo::hypercube_full_indices, sizeof(BlockInfo::hypercube_full_indices));

	renderer.setMesh(&mesh);
}

bool ItemJetpack::isDeadly() { return true; }
uint32_t ItemJetpack::getStackLimit() { return 1; }

nlohmann::json ItemJetpack::saveAttributes() {
	return { { "fuelLevel", fuelLevel }, { "isFuelDeadly", isFuelDeadly}, { "isSelectedFuelDeadly", isSelectedFuelDeadly} };
}

std::unique_ptr<Item> ItemJetpack::clone() {
	auto result = std::make_unique<ItemJetpack>();

	result->fuelLevel = fuelLevel;
	result->isFuelDeadly =isFuelDeadly;
	result->isSelectedFuelDeadly = isSelectedFuelDeadly;

	return result;
}

$hook(void, Player, update,World* world, double dt, EntityPlayer* entityPlayer) {
	original(self, world, dt,entityPlayer);
	ItemJetpack* jetpack;
	jetpack = dynamic_cast<ItemJetpack*>(self->hotbar.getSlot(self->hotbar.selectedIndex)->get());
	if (!jetpack) jetpack = dynamic_cast<ItemJetpack*>(self->equipment.getSlot(0)->get());
	if (!jetpack) return;
	jetpack->handleFlight(self);
}

// instantiating jetpack item
$hookStatic(std::unique_ptr<Item>, Item, instantiateItem, const stl::string& itemName, uint32_t count, const stl::string& type, const nlohmann::json& attributes) {

	if (itemName != "Jetpack")
		return original(itemName, count, type, attributes);

	auto result = std::make_unique<ItemJetpack>();

	result->fuelLevel = (float)attributes["fuelLevel"];
	result->isFuelDeadly=(bool)attributes["isFuelDeadly"];
	result->isSelectedFuelDeadly = (bool)attributes["isSelectedFuelDeadly"];
	result->count = count;

	return result;
}