#pragma once
#include "core/engine.hpp"
#include "griditem.hpp"
#include "inventory.hpp"
class GameState;

//////////////////////////////////////////////////////
class Unit : public GridItem
{
public:
    Unit(int x = 0, int y = 0);
    Unit(Texture * tex, int w, int h, int x = 0, int y = 0);
    virtual ~Unit() {}

    void SetGameState(GameState * g) { game = g; }
	void SetMaxHealth(int health);
	void SetItemDrop(ItemType item);

    uint16_t getMovesLeft() const { return m_movesLeft; }
    bool getAttacked() const { return m_attacked; }

    virtual void StartTurn();

    virtual bool IsPlayer() const { return false; }
	virtual bool Collidable() const override { return m_currentHealth > 0; }
	virtual bool ShouldRender() const override { return m_currentHealth > 0; }

    void RenderHealth(int cam_x, int cam_y, int tilesize) const;

	int GetCurrentHealth() const { return m_currentHealth; }
	int GetMaxHealth() const { return m_maxHealth; }
    void TakeDamage(int damage);

protected:

    // Unit properties
    int m_maxHealth;
    int m_currentHealth;

    // Unit states
    uint16_t m_movesLeft;
    bool m_attacked;
	ItemType m_itemDrop;

    // Store a copy of the game
    GameState * game;
};