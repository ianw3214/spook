#include "game.hpp"

#include "resource.hpp"

GameState::GameState()
    : m_map_width(10)
    , m_map_height(10)
	, m_state(State::GAME)
    , m_camera_x(0)
    , m_camera_y(0)
    , m_selected(nullptr)
    , m_playerTurn(false)
    , m_panning(false)
	, m_craftingIndex(ItemType::WOOD)
{
    for (unsigned int i = 0; i < m_map_width; ++i)
    {
        for (unsigned int j = 0; j < m_map_height; ++j)
        {
            m_tilemap.push_back({0});
        }
    }

    // Initialize textures
    createFont("ui30", "res/Munro.ttf", 30);
    m_tile_texture = new Texture("res/tile.png");
    m_ui_texture = new Texture("res/ui_base.png");
    m_white_overlay = new Texture("res/white_overlay.png");
    m_end_turn = new Texture("res/endturn.png");
	m_crafting = new Texture("res/crafting.png");
	m_craftingBackground = new Texture("res/crafting_bg.png");
	m_craftingLeft = new Texture("res/crafting_left.png");
	m_craftingRight = new Texture("res/crafting_right.png");

    // Initialize some units
    {
        Player * player = new Player(new Texture("res/IRA.png"), 64, 128, 0, 0);
        player->SetGameState(this);
        m_items.push_back(player);
        m_units.push_back(player);
        m_players.push_back(player);   
    }
    {
        Player * player = new Player(6, 7);
        player->SetGameState(this);
        m_items.push_back(player);
        m_units.push_back(player);
        m_players.push_back(player);   
    }
    {
        Player * player = new Player(4, 3);
        player->SetGameState(this);
        m_items.push_back(player);
        m_units.push_back(player);
        m_players.push_back(player);   
    }
    {
        AI * unit = new AI(new Texture("res/enemy.png"), 64, 128, 2, 2);
        unit->SetGameState(this);
		unit->SetStrategy(AI::Strategy::HOSTILE_DUMB);
        m_items.push_back(unit);
        m_units.push_back(unit);
		m_AIs.push_back(unit);
    }
    {
		AI * unit = new AI(new Texture("res/enemy.png"), 64, 128, 3, 3);
        unit->SetGameState(this);
		unit->SetStrategy(AI::Strategy::HOSTILE_DUMB);
        m_items.push_back(unit);
        m_units.push_back(unit);
		m_AIs.push_back(unit);
    }
    {
		AI * unit = new AI(new Texture("res/enemy.png"), 64, 128, 4, 4);
        unit->SetGameState(this);
		unit->SetStrategy(AI::Strategy::HOSTILE_DUMB);
        m_items.push_back(unit);
        m_units.push_back(unit);
		m_AIs.push_back(unit);
    }
	{
		AI * unit = new AI(new Texture("res/pig.png"), 90, 90, 4, 2);
		unit->SetGameState(this);
		unit->SetMaxHealth(2);
		unit->SetItemDrop(ItemType::MEAT);
		m_items.push_back(unit);
		m_units.push_back(unit);
		m_AIs.push_back(unit);
	}
    {
        Resource * res = new Resource(3, 8);
		res->SetGameRef(this);
        m_items.push_back(res);
        m_interactables.push_back(res);
    }
    {
		Resource * res = new Resource(6, 3);
		res->SetGameRef(this);
        m_items.push_back(res);
        m_interactables.push_back(res);
    }

    StartTurn();
    tempDelta = 0;
}

GameState::~GameState()
{
    
}

void GameState::init()
{

}
void GameState::cleanup()
{

}

void GameState::update()
{
    // Temporary time delaying code
    if (!m_playerTurn)
    {
        if (tempDelta > 3000)
        {
            tempDelta = 0;
            StartTurn();
        }
        else 
        {
            tempDelta += delta;
        }
    }
    if (keyPressed(SDL_SCANCODE_ESCAPE)) {
        exit();
    }
    if (leftMousePressed())
    {
        bool handled = false;
        // Handle inputs for the current selected unit
        if (m_selected)
        {
            if (m_playerTurn && m_selected->HandleClick(getMouseX(), getMouseY(), m_camera_x, m_camera_y, kTilesize))
            {
                handled = true;
            }
        }
        if (!handled)
        {
            // Handle UI
            Vec2 mouse(getMouseX(), getMouseY());
			Math::Rectangle craftingButton(0, 400, 160, 70);
			if (Math::isColliding(mouse, craftingButton))
			{
				m_state = State::CRAFTING;
				handled = true;
			}
            Math::Rectangle endButton(0, 600, 180, 80);
            if (m_playerTurn && Math::isColliding(mouse, endButton))
            {
                // End the turn here
                EndTurn();
                handled = true;
            }
            if (!handled)
            {
				if (m_state == State::CRAFTING)
				{
					constexpr int x = (1280 - 220) / 2;
					constexpr int y = (620 - 52) / 2;

					// Left and right buttons
					Math::Rectangle left(x - 52 - 5, y, 52, 52);
					if (Math::isColliding(mouse, left)) 
					{
						// Find the next valid crafting recipe and render it
						ItemDatabase::Recipe * recipe;
						do {
							m_craftingIndex = static_cast<ItemType>(static_cast<int>(m_craftingIndex) - 1);
							if (m_craftingIndex <= ItemType::NONE)
							{
								m_craftingIndex = static_cast<ItemType>(static_cast<int>(ItemType::COUNT) - 1);
							}
							recipe = &(ItemDatabase::recipes[m_craftingIndex]);
						} while (recipe->item1 == ItemType::NONE && recipe->item2 == ItemType::NONE && recipe->item3 == ItemType::NONE);
						handled = true;
					}
					Math::Rectangle right(x + 220 + 5, y, 52, 52);
					if (Math::isColliding(mouse, right))
					{
						// Find the next valid crafting recipe and render it
						ItemDatabase::Recipe * recipe;
						do {
							m_craftingIndex = static_cast<ItemType>(static_cast<int>(m_craftingIndex) + 1);
							if (m_craftingIndex >= ItemType::COUNT)
							{
								m_craftingIndex = static_cast<ItemType>(0);
							}
							recipe = &(ItemDatabase::recipes[m_craftingIndex]);

						} while (recipe->item1 == ItemType::NONE && recipe->item2 == ItemType::NONE && recipe->item3 == ItemType::NONE);
						handled = true;
					}

					Math::Rectangle craft(x + 178, y + 10, 32, 32);
					if (Math::isColliding(mouse, craft))
					{
						if (m_inventory.HasItemsFor(m_craftingIndex))
						{
							m_inventory.Craft(m_craftingIndex);
						}
						else
						{
							// TODO: User feedback for not enough items
						}
						handled = true;
					}
					// Exit the crafting screen the user clicks somewhere else
					if (!handled)
					{
						Math::Rectangle area(x, y, 220, 52);
						if (!Math::isColliding(mouse, area))
						{
							m_state = State::GAME;
							handled = true;
						}
					}
				}
                int mouseTileX = (getMouseX() + m_camera_x) / kTilesize;
                int mouseTileY = (getMouseY() + m_camera_y) / kTilesize;
                for (Player * player : m_players)
                {
                    if (player->getX() == mouseTileX && player->getY() == mouseTileY)
                    {
                        if (m_selected) m_selected->Deselect();
                        m_selected = player;
                        m_selected->Select();
                        handled = true;
                        break;
                    }
                }
                if (!handled)
                {
                    if (m_selected) m_selected->Deselect();
                    m_selected = nullptr;
                    if (!m_panning)
                    {
                        m_pan_start_x = getMouseX();
                        m_pan_start_y = getMouseY();
                        m_pan_start_cam_x = m_camera_x;
                        m_pan_start_cam_y = m_camera_y;
                        m_panning = true;
                    }
                }
            }
        }
    }
    if (leftMouseReleased())
    {
        m_panning = false;
    }
    if (leftMouseHeld())
    {
        if (m_panning == true)
        {
            // Calculate the camera difference and apply it
            m_camera_x = m_pan_start_cam_x + m_pan_start_x - getMouseX();
            m_camera_y = m_pan_start_cam_y + m_pan_start_y - getMouseY();
        }
    }
}

void GameState::render()
{
    for (unsigned int y = 0; y < m_map_height; ++y)
    {
        for (unsigned int x = 0; x < m_map_width; ++x)
        {
            const Tile& tile = m_tilemap[y * m_map_height + x];
            if (tile.index == 0)
            {
                m_tile_texture->render(
                    x * kTilesize - m_camera_x, 
                    y * kTilesize - m_camera_y, 
                    kTilesize,
                    kTilesize
                );
            }
        }
    }

    // Render overlays when needed
    if (m_playerTurn)
    {
        if (m_selected)
        {
            if (m_selected->getState() == Player::InputState::MOVE && m_selected->getMovesLeft() > 0)
            {
                // Just assume the player can only melee attack for now
                int x = m_selected->getX() * kTilesize;
                int y = m_selected->getY() * kTilesize;
                
                if (!checkOccupied(m_selected->getX() - 1, m_selected->getY()))
                {
                    m_white_overlay->render(
                    x - kTilesize - m_camera_x,
                    y - m_camera_y,
                    kTilesize,
                    kTilesize
                );
                }
                if (!checkOccupied(m_selected->getX() + 1, m_selected->getY()))
                {
                    m_white_overlay->render(
                    x + kTilesize - m_camera_x,
                    y - m_camera_y,
                    kTilesize,
                    kTilesize
                );
                }
                if (!checkOccupied(m_selected->getX(), m_selected->getY() - 1))
                {
                    m_white_overlay->render(
                    x - m_camera_x,
                    y - kTilesize - m_camera_y,
                    kTilesize,
                    kTilesize
                );
                }
                if (!checkOccupied(m_selected->getX(), m_selected->getY() + 1))
                {
                    m_white_overlay->render(
                    x - m_camera_x,
                    y + kTilesize - m_camera_y,
                    kTilesize,
                    kTilesize
                );
                }
            }
            if (m_selected->getState() == Player::InputState::ATTACK && !m_selected->getAttacked())
            {
                // Just assume the player can only melee attack for now
                int x = m_selected->getX() * kTilesize;
                int y = m_selected->getY() * kTilesize;
                
                if (getUnitAt(m_selected->getX() - 1, m_selected->getY()))
                {
                    m_white_overlay->render(
                        x - kTilesize - m_camera_x,
                        y - m_camera_y,
                        kTilesize,
                        kTilesize
                    );
                }
                if (getUnitAt(m_selected->getX() + 1, m_selected->getY()))
                {
                    m_white_overlay->render(
                        x + kTilesize - m_camera_x,
                        y - m_camera_y,
                        kTilesize,
                        kTilesize
                    );   
                }
                if (getUnitAt(m_selected->getX(), m_selected->getY() - 1))
                {
                    m_white_overlay->render(
                        x - m_camera_x,
                        y - kTilesize - m_camera_y,
                        kTilesize,
                        kTilesize
                    );   
                }
                if (getUnitAt(m_selected->getX(), m_selected->getY() + 1))
                {
                    m_white_overlay->render(
                        x - m_camera_x,
                        y + kTilesize - m_camera_y,
                        kTilesize,
                        kTilesize
                    );   
                }
            }
			if (m_selected->getState() == Player::InputState::INTERACT)
			{
				// Player can only interact with neighboor tiles
				int x = m_selected->getX() * kTilesize;
				int y = m_selected->getY() * kTilesize;

				if (getInteractable(m_selected->getX() - 1, m_selected->getY()))
				{
					m_white_overlay->render(
						x - kTilesize - m_camera_x,
						y - m_camera_y,
						kTilesize,
						kTilesize
					);
				}
				if (getInteractable(m_selected->getX() + 1, m_selected->getY()))
				{
					m_white_overlay->render(
						x + kTilesize - m_camera_x,
						y - m_camera_y,
						kTilesize,
						kTilesize
					);
				}
				if (getInteractable(m_selected->getX(), m_selected->getY() - 1))
				{
					m_white_overlay->render(
						x - m_camera_x,
						y - kTilesize - m_camera_y,
						kTilesize,
						kTilesize
					);
				}
				if (getInteractable(m_selected->getX(), m_selected->getY() + 1))
				{
					m_white_overlay->render(
						x - m_camera_x,
						y + kTilesize - m_camera_y,
						kTilesize,
						kTilesize
					);
				}
			}
        }
    }

    // Draw a grid overlay for EZ UX
    for (unsigned int row = 1; row < m_map_height; ++row)
    {
        QcE::get_instance()->drawLine(
            0 - m_camera_x,
            row * kTilesize - m_camera_y, 
            m_map_width * kTilesize - m_camera_x, 
            row * kTilesize - m_camera_y,
            {0, 0, 0, 255}
        );
    }
    for (unsigned int col = 1; col < m_map_width; ++col)
    {
        QcE::get_instance()->drawLine(
            col * kTilesize - m_camera_x, 
            0 - m_camera_y,
            col * kTilesize - m_camera_x,
            m_map_height * kTilesize - m_camera_y,
            {0, 0, 0, 255}
        );
    }

    // Draw units
    for (const GridItem * item : m_items)
    {
        item->Render(m_camera_x, m_camera_y, kTilesize);
    }
    // Draw unit health
    for (const Unit * unit : m_units)
    {
        unit->RenderHealth(m_camera_x, m_camera_y, kTilesize);
    }
    // Render UI
    if (m_playerTurn)
    {
        if (m_selected && m_selected->getState() == Player::InputState::NONE)
        {
            m_selected->RenderUI(m_camera_x, m_camera_y, kTilesize, m_ui_texture);
        }
    }
	m_crafting->render(0, 400, 160, 70);
    m_end_turn->render(0, 600, 180, 80);
	m_inventory.Render();

	if (m_state == State::CRAFTING)
	{
		constexpr int x = (1280 - 220) / 2;
		constexpr int y = (620 - 52) / 2;
		m_craftingBackground->render(x, y);

		// Left and right buttons
		m_craftingLeft->render(x - 52 - 5, y);
		m_craftingRight->render(x + 220 + 5, y);

		// Find the next valid crafting recipe and render it
		ItemDatabase::Recipe * recipe = &(ItemDatabase::recipes[m_craftingIndex]);
		while (recipe->item1 == ItemType::NONE && recipe->item2 == ItemType::NONE && recipe->item3 == ItemType::NONE)
		{
			m_craftingIndex = static_cast<ItemType>(static_cast<int>(m_craftingIndex) + 1);
			if (m_craftingIndex >= ItemType::COUNT)
			{
				m_craftingIndex = static_cast<ItemType>(0);
			}
			recipe = &(ItemDatabase::recipes[m_craftingIndex]);
		}
		if (recipe->item1 != ItemType::NONE)
		{
			Texture texture(ItemDatabase::items[recipe->item1].icon);
			texture.render(x + 10, y + 10);
		}
		if (recipe->item2 != ItemType::NONE)
		{
			Texture texture(ItemDatabase::items[recipe->item2].icon);
			texture.render(x + 52, y + 10);
		}
		if (recipe->item3 != ItemType::NONE)
		{
			Texture texture(ItemDatabase::items[recipe->item3].icon);
			texture.render(x + 94, y + 10);
		}
		Texture texture(ItemDatabase::items[m_craftingIndex].icon);
		texture.render(x + 178, y + 10);
	}
}

bool GameState::checkOccupied(unsigned int x, unsigned int y) const
{
    if (x < 0 || x >= m_map_width || y < 0 || y > m_map_height) return true;
    for (const GridItem * const item : m_items)
    {
        if (item->getX() == x && item->getY() == y) return true;
    }
    return false;
}

Unit * GameState::getUnitAt(unsigned int x, unsigned int y)
{
    for (Unit * unit : m_units)
    {
        if (unit->getX() == x && unit->getY() == y)
        {
            return unit;
        }
    }
    return nullptr;
}

Interactable * GameState::getInteractable(unsigned int x, unsigned int y)
{
	for (Interactable * interactable : m_interactables)
	{
		if (interactable->getX() == x && interactable->getY() == y)
		{
			return interactable->CanInteract() ? interactable : nullptr;
		}
	}
	return nullptr;
}

void GameState::StartTurn()
{
    m_playerTurn = true;
}

void GameState::EndTurn()
{
    m_playerTurn = false;
    for (Player * player : m_players)
    {
        player->StartTurn();
    }
	// HAVE THE AIs Take their turn
	for (AI * ai : m_AIs)
	{
		ai->DoMoves();
	}
}