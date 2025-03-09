//#define DEBUG_CONSOLE // Uncomment this if you want a debug console to start. You can use the Console class to print. You can use Console::inStrings to get input.

#include <4dm.h>
#include "libs/auilib/auilib.h"
#include "4DKeyBinds.h"
#include "ItemJetpack.h"

using namespace fdm;

// Initialize the DLLMain
initDLL

std::vector<std::string> materialNames = {
	"Biofuel",
	"Deadly Fuel",
	"Deadly Casing",
	"Iron Plate",
	"Solenoid Wire",
	"Gyroscope",
	"Jetpack Controller"
};
std::vector<std::string> toolNames = {
	"Jetpack"
};

aui::BarIndicator fuelIndicator{};
QuadRenderer qr{};
TexRenderer fuelBackgroundRenderer;
TexRenderer fuelRenderer;
gui::Text fuelCountText;
gui::Text jetpackModeText;
gui::Interface ui;
FontRenderer font{};

//Initialize UI
void viewportCallback(void* user, const glm::ivec4& pos, const glm::ivec2& scroll)
{
	GLFWwindow* window = (GLFWwindow*)user;

	// update the render viewport
	int wWidth, wHeight;
	glfwGetWindowSize(window, &wWidth, &wHeight);
	glViewport(pos.x, wHeight - pos.y - pos.w, pos.z, pos.w);

	// create a 2D projection matrix from the specified dimensions and scroll position
	glm::mat4 projection2D = glm::ortho(0.0f, (float)pos.z, (float)pos.w, 0.0f, -1.0f, 1.0f);
	projection2D = glm::translate(projection2D, { scroll.x, scroll.y, 0 });

	// update all 2D shaders
	const Shader* textShader = ShaderManager::get("textShader");
	textShader->use();
	glUniformMatrix4fv(glGetUniformLocation(textShader->id(), "P"), 1, GL_FALSE, &projection2D[0][0]);

	const Shader* tex2DShader = ShaderManager::get("tex2DShader");
	tex2DShader->use();
	glUniformMatrix4fv(glGetUniformLocation(tex2DShader->id(), "P"), 1, GL_FALSE, &projection2D[0][0]);

	const Shader* quadShader = ShaderManager::get("quadShader");
	quadShader->use();
	glUniformMatrix4fv(glGetUniformLocation(quadShader->id(), "P"), 1, GL_FALSE, &projection2D[0][0]);
}
$hook(void, StateGame, init, StateManager& s)
{
	original(self, s);

	font = { ResourceManager::get("pixelFont.png"), ShaderManager::get("textShader") };

	qr.shader = ShaderManager::get("quadShader");
	qr.init();

	fuelIndicator.setSize(10, 50);
	fuelIndicator.isHorizontal = false;
	fuelIndicator.showText = false;
	fuelIndicator.setOutlineColor(glm::vec4{ 1,1,1,0.75f });

	fuelBackgroundRenderer.texture = ItemBlock::tr->texture;
	fuelBackgroundRenderer.shader = ShaderManager::get("tex2DShader");
	fuelBackgroundRenderer.init();

	fuelRenderer.texture = ResourceManager::get("assets/Items.png", true);
	fuelRenderer.shader = ShaderManager::get("tex2DShader");
	fuelRenderer.init();

	fuelCountText.size = 2;
	fuelCountText.text = "0";
	fuelCountText.shadow = true;

	jetpackModeText.size = 2;
	jetpackModeText.text = "O";
	jetpackModeText.shadow = true;

	// initialize the Interface
	ui = gui::Interface{ s.window };
	ui.viewportCallback = viewportCallback;
	ui.viewportUser = s.window;
	ui.font = &font;
	ui.qr = &qr;

	ui.addElement(&fuelCountText);
	ui.addElement(&jetpackModeText);
	ui.addElement(&fuelIndicator);
}
// Render UI
$hook(void, Player, renderHud, GLFWwindow* window) {
	original(self, window);

	static bool isJetpackOffhand;
	isJetpackOffhand = false;

	if (!&StateGame::instanceObj->player || StateGame::instanceObj->player.inventoryManager.isOpen()) return;
	ItemJetpack* jetpack;
	jetpack = dynamic_cast<ItemJetpack*>(StateGame::instanceObj->player.hotbar.getSlot(StateGame::instanceObj->player.hotbar.selectedIndex)->get());
	if (!jetpack) {
		jetpack = dynamic_cast<ItemJetpack*>(StateGame::instanceObj->player.equipment.getSlot(0)->get());
		isJetpackOffhand = true;
	}
	if (!jetpack) return;

	glDisable(GL_DEPTH_TEST);

	int width, height;
	glfwGetWindowSize(window, &width, &height);

	
	int posX, posY;
	if (!isJetpackOffhand) {
		posX = self->hotbar.renderPos.x + 68 /*imperfect reality*/ + 34 * ((self->hotbar.selectedIndex + 1) % 2);
		posY = self->hotbar.renderPos.y + (self->hotbar.selectedIndex * 56);
	}
	else {
		posX = self->hotbar.renderPos.x + 78 + 34 * ((9) % 2);
		posY = self->hotbar.renderPos.y + (8 * 56) + 6;
	}


	fuelIndicator.setFill(jetpack->fuelLevel);
	fuelIndicator.offsetX(width / 2 + 30);
	fuelIndicator.offsetY(height / 2 - 25);
	fuelIndicator.setFillColor(jetpack->isFuelDeadly ?
		glm::vec4{ 147.0f / 255.0f,55.0f / 255.0f,118.0f / 255.0f,0.75f } :
		glm::vec4{ 82.0f / 255.0f,144.0f / 255.0f,40.0f / 255.0f,0.75f });

	fuelBackgroundRenderer.setPos(posX, posY, 72, 72);
	fuelBackgroundRenderer.setClip(0, 0, 36, 36);
	fuelBackgroundRenderer.setColor(1, 1, 1, 0.4);
	fuelBackgroundRenderer.render();

	fuelRenderer.setPos(posX, posY, 72, 72);
	fuelRenderer.setClip((jetpack->isSelectedFuelDeadly) * 36, 0, 36, 36);
	if (jetpack->getSelectedFuelCount(self->inventoryAndEquipment) > 0)
		fuelRenderer.setColor(1, 1, 1, 1);
	else
		fuelRenderer.setColor(.2, .2, .2, 1);
	fuelRenderer.render();

	fuelCountText.text = std::to_string(jetpack->getSelectedFuelCount(self->inventoryAndEquipment));
	fuelCountText.offsetX(posX + 45);
	fuelCountText.offsetY(posY + 45);

	switch (jetpack->flightMode) {
	case ItemJetpack::Flight:
		jetpackModeText.text = "F";
		break;
	case ItemJetpack::Hover:
		jetpackModeText.text = "H";
		break;
	default:
		jetpackModeText.text = "O";
		break;
	}
	jetpackModeText.offsetX(posX+45);
	jetpackModeText.offsetY(posY+11);

	ui.render();
	
	glEnable(GL_DEPTH_TEST);

}

// Item slot material
$hook(void, ItemMaterial, render, const glm::ivec2& pos)
{
	int index = std::find(materialNames.begin(), materialNames.end(), self->name) - materialNames.begin();
	if (index == materialNames.size())
		return original(self, pos);

	TexRenderer& tr = *ItemTool::tr; // or TexRenderer& tr = ItemTool::tr; after 0.3
	const Tex2D* ogTex = tr.texture; // remember the original texture

	tr.texture = ResourceManager::get("assets/Materials.png", true); // set to custom texture
	tr.setClip(index * 36, 0, 36, 36);
	tr.setPos(pos.x, pos.y, 70, 72);
	tr.render();

	tr.texture = ogTex; // return to the original texture

}

$hook(bool, ItemMaterial, isDeadly)
{
	if (self->name == "Deadly Fuel" || self->name == "Deadly Casing")
		return true;
	return original(self);
}

// add recipes
$hookStatic(void, CraftingMenu, loadRecipes)
{
	static bool recipesLoaded = false;

	if (recipesLoaded) return;

	recipesLoaded = true;

	original();

	CraftingMenu::recipes->push_back(
		nlohmann::json{
		{"recipe", {{{"name", "Glass"}, {"count", 3}}}},
		{"result", {{"name", "Klein Bottle"}, {"count", 1}}}
		}
	);

	CraftingMenu::recipes->push_back(
		nlohmann::json{
		{"recipe", {{{"name", "Klein Bottle"}, {"count", 1}},{{"name", "Leaf"}, {"count", 3}},{{"name", "Wood"}, {"count", 1}}}},
		{"result", {{"name", "Biofuel"}, {"count", 1}}}
		}
	);

	CraftingMenu::recipes->push_back(
		nlohmann::json{
		{"recipe", {{{"name", "Klein Bottle"}, {"count", 1}},{{"name", "Deadly Ore"}, {"count", 1}}}},
		{"result", {{"name", "Deadly Fuel"}, {"count", 1}}}
		}
	);

	CraftingMenu::recipes->push_back(
		nlohmann::json{
		{"recipe", {{{"name", "Iron Bars"}, {"count", 3}}}},
		{"result", {{"name", "Iron Plate"}, {"count", 1}}}
		}
	);

	CraftingMenu::recipes->push_back(
		nlohmann::json{
		{"recipe", {{{"name", "Solenoid Bars"}, {"count", 2}}}},
		{"result", {{"name", "Solenoid Wire"}, {"count", 1}}}
		}
	);

	CraftingMenu::recipes->push_back(
		nlohmann::json{
		{"recipe", {{{"name", "Deadly Bars"}, {"count", 4}}}},
		{"result", {{"name", "Deadly Casing"}, {"count", 1}}}
		}
	);

	CraftingMenu::recipes->push_back(
		nlohmann::json{
		{"recipe", {{{"name", "Iron Bars"}, {"count", 2}},{{"name", "Compass"}, {"count", 1}}}},
		{"result", {{"name", "Gyroscope"}, {"count", 1}}}
		}
	);

	CraftingMenu::recipes->push_back(
		nlohmann::json{
		{"recipe", {{{"name", "Gyroscope"}, {"count", 1}},{{"name", "Solenoid Wire"}, {"count", 2}},{{"name", "Iron Plate"}, {"count", 1}}}},
		{"result", {{"name", "Jetpack Controller"}, {"count", 1}}}
		}
	);

	CraftingMenu::recipes->push_back(
		nlohmann::json{
		{"recipe", {
			{{"name", "Jetpack Controller"},{"count", 1}},
			{{"name", "Solenoid Wire"}, {"count", 1}},
			{{"name", "Iron Plate"}, {"count", 1}},
			{{"name", "Deadly Casing"}, {"count", 2}}
		}},
		{"result", {{"name", "Jetpack"}, {"count", 1}}}
		}
	);
}

// Initialize stuff
void initItemNAME()
{
	for (int i = 0;i < materialNames.size(); i++)
		(*Item::blueprints)[materialNames[i]] =
	{
		{ "type", "material" },
		{ "baseAttributes", nlohmann::json::object() } // no attributes
	};

	for (int i = 0;i < toolNames.size(); i++)
		(*Item::blueprints)[toolNames[i]] =
	{
		{ "type", "jetpack" },
		{ "baseAttributes", { {"fuelLevel", 0.0f},{"isFuelDeadly", false},{"isSelectedFuelDeadly", false}}}
	};
}

$hook(void, StateIntro, init, StateManager& s)
{
	original(self, s);

	// initialize opengl stuff
	glewExperimental = true;
	glewInit();
	glfwInit();

	initItemNAME();

	ItemJetpack::rendererInit();
}

//Keybinds
void changeFuel(GLFWwindow* window, int action, int mods)
{
	Player* player = &StateGame::instanceObj->player;
	if (action != GLFW_PRESS || player == nullptr || player->inventoryManager.isOpen()) return;

	ItemJetpack* jetpack;
	jetpack = dynamic_cast<ItemJetpack*>(player->hotbar.getSlot(player->hotbar.selectedIndex)->get());
	if (!jetpack) jetpack = dynamic_cast<ItemJetpack*>(player->equipment.getSlot(0)->get());
	if (!jetpack) return;
	jetpack->isSelectedFuelDeadly = !jetpack->isSelectedFuelDeadly;
}
void changeFlightMode(GLFWwindow* window, int action, int mods)
{
	Player* player = &StateGame::instanceObj->player;
	if (action != GLFW_PRESS || player == nullptr || player->inventoryManager.isOpen()) return;

	ItemJetpack* jetpack;
	jetpack = dynamic_cast<ItemJetpack*>(player->hotbar.getSlot(player->hotbar.selectedIndex)->get());
	if (!jetpack) jetpack = dynamic_cast<ItemJetpack*>(player->equipment.getSlot(0)->get());
	if (!jetpack) return;
	jetpack->flightMode = (ItemJetpack::FlightMode)(((int)jetpack->flightMode+1)%3);
}

void flushFuelTank(GLFWwindow* window, int action, int mods)
{
	Player* player = &StateGame::instanceObj->player;
	if (action != GLFW_PRESS || player == nullptr || player->inventoryManager.isOpen()) return;

	ItemJetpack* jetpack;
	jetpack = dynamic_cast<ItemJetpack*>(player->hotbar.getSlot(player->hotbar.selectedIndex)->get());
	if (!jetpack) jetpack = dynamic_cast<ItemJetpack*>(player->equipment.getSlot(0)->get());
	if (!jetpack) return;
	jetpack->fuelLevel = 0.0f;
}

// I was trying to make coordinates render when holding a jetpack since it is made using it
// It did not render coordinates, instead it locked right click ._.
/*
$hook(bool, Player, isHoldingCompass) {
	if (original(self)) return true;
	Player* player = &StateGame::instanceObj->player;
	if (player == nullptr || player->inventoryManager.isOpen()) return false;

	ItemJetpack* jetpack;
	jetpack = dynamic_cast<ItemJetpack*>(player->hotbar.getSlot(player->hotbar.selectedIndex)->get());
	if (!jetpack) jetpack = dynamic_cast<ItemJetpack*>(player->equipment.getSlot(0)->get());
	return jetpack;
}
*/
$exec
{
	KeyBinds::addBind("Jetpack Mod", "Change fuel", glfw::Keys::R, KeyBindsScope::PLAYER, changeFuel);
	KeyBinds::addBind("Jetpack Mod", "Change flight mode", glfw::Keys::F, KeyBindsScope::PLAYER, changeFlightMode);
	KeyBinds::addBind("Jetpack Mod", "Flush fuel tank", glfw::Keys::Z, KeyBindsScope::PLAYER, flushFuelTank);
}