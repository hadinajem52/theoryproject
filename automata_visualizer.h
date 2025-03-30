#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <memory>
#include "lexer.h"
#include "parser.h"

class AutomataVisualizer {
public:
    AutomataVisualizer();
    ~AutomataVisualizer();
    
    void initialize();
    void visualizeLexer(const Lexer& lexer);
    void visualizeParser(const Parser& parser);
    void run();
    
private:
    sf::RenderWindow window;
    sf::Font font;
    
    // Automaton visualization data
    struct Node {
        sf::CircleShape shape;
        sf::Text label;
        bool isActive;
    };
    
    struct Edge {
        sf::VertexArray line;
        sf::Text label;
        sf::CircleShape arrow;
    };
    
    std::vector<Node> lexerNodes;
    std::vector<Edge> lexerEdges;
    std::vector<Node> parserNodes;
    std::vector<Edge> parserEdges;
    
    // Current visualization mode
    enum Mode { LEXER, PARSER } currentMode;
    
    // Helper methods
    void drawAutomaton();
    void layoutNodes(std::vector<Node>& nodes);
    void createEdges(const std::vector<Node>& nodes, 
                   std::vector<Edge>& edges, 
                   const std::vector<std::pair<int, int>>& connections,
                   const std::vector<std::string>& labels);
    void handleEvents();
    void switchMode();
};
