#include "automata_visualizer.h"
#include <cmath>
#include <algorithm>
#include <iostream>

// Constants for visualization
const float NODE_RADIUS = 30.0f;
const float WINDOW_WIDTH = 1024.0f;
const float WINDOW_HEIGHT = 768.0f;
const sf::Color ACTIVE_COLOR(0, 255, 0);
const sf::Color INACTIVE_COLOR(100, 100, 100);
const sf::Color TEXT_COLOR(255, 255, 255);
const sf::Color EDGE_COLOR(200, 200, 200);
const sf::Color BACKGROUND_COLOR(50, 50, 50);

AutomataVisualizer::AutomataVisualizer() : currentMode(LEXER) {
}

AutomataVisualizer::~AutomataVisualizer() {
    if (window.isOpen()) {
        window.close();
    }
}

void AutomataVisualizer::initialize() {
    // Create window
    window.create(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Automaton Visualizer");
    window.setFramerateLimit(60);
    
    // Load font - try resources directory first
    if (!font.loadFromFile("resources/arial.ttf")) {
        std::cerr << "Error loading font from resources directory!" << std::endl;
        // Try current directory
        if (!font.loadFromFile("arial.ttf")) {
            std::cerr << "Error loading font from current directory!" << std::endl;
            // Try to load a system font as fallback
            if (!font.loadFromFile("C:\\Windows\\Fonts\\arial.ttf")) {
                std::cerr << "Failed to load any font!" << std::endl;
            }
        }
    }
}

void AutomataVisualizer::visualizeLexer(const Lexer& lexer) {
    // Get lexer states and transitions
    auto states = lexer.getStates();
    auto transitions = lexer.getTransitions();
    auto currentState = lexer.getCurrentState();
    
    // Clear existing nodes and edges
    lexerNodes.clear();
    lexerEdges.clear();
    
    // Create nodes for each state
    for (const auto& state : states) {
        Node node;
        node.shape.setRadius(NODE_RADIUS);
        node.shape.setFillColor(INACTIVE_COLOR);
        node.shape.setOrigin(NODE_RADIUS, NODE_RADIUS);
        
        node.label.setFont(font);
        node.label.setString(state.name);
        node.label.setCharacterSize(16);
        node.label.setFillColor(TEXT_COLOR);
        
        // Center the text in the node
        sf::FloatRect textBounds = node.label.getLocalBounds();
        node.label.setOrigin(textBounds.width / 2.0f, textBounds.height / 2.0f);
        
        // Check if this is the current state
        node.isActive = (state.type == currentState.type);
        if (node.isActive) {
            node.shape.setFillColor(ACTIVE_COLOR);
        }
        
        lexerNodes.push_back(node);
    }
    
    // Layout the nodes in a circular pattern
    layoutNodes(lexerNodes);
    
    // Create edges for transitions
    std::vector<std::pair<int, int>> connections;
    std::vector<std::string> labels;
    
    for (const auto& transition : transitions) {
        // Find the indices of the source and target states
        int sourceIdx = -1, targetIdx = -1;
        for (size_t i = 0; i < states.size(); ++i) {
            if (states[i].type == transition.from.type) sourceIdx = i;
            if (states[i].type == transition.to.type) targetIdx = i;
        }
        
        if (sourceIdx >= 0 && targetIdx >= 0) {
            connections.emplace_back(sourceIdx, targetIdx);
            labels.push_back(transition.condition);
        }
    }
    
    createEdges(lexerNodes, lexerEdges, connections, labels);
}

void AutomataVisualizer::visualizeParser(const Parser& parser) {
    // Get parser states and transitions
    auto states = parser.getStates();
    auto transitions = parser.getTransitions();
    auto currentState = parser.getCurrentState();
    
    // Clear existing nodes and edges
    parserNodes.clear();
    parserEdges.clear();
    
    // Create nodes for each state
    for (const auto& state : states) {
        Node node;
        node.shape.setRadius(NODE_RADIUS);
        node.shape.setFillColor(INACTIVE_COLOR);
        node.shape.setOrigin(NODE_RADIUS, NODE_RADIUS);
        
        node.label.setFont(font);
        node.label.setString(state.name);
        node.label.setCharacterSize(16);
        node.label.setFillColor(TEXT_COLOR);
        
        // Center the text in the node
        sf::FloatRect textBounds = node.label.getLocalBounds();
        node.label.setOrigin(textBounds.width / 2.0f, textBounds.height / 2.0f);
        
        // Check if this is the current state
        node.isActive = (state.type == currentState.type);
        if (node.isActive) {
            node.shape.setFillColor(ACTIVE_COLOR);
        }
        
        parserNodes.push_back(node);
    }
    
    // Layout the nodes in a circular pattern
    layoutNodes(parserNodes);
    
    // Create edges for transitions
    std::vector<std::pair<int, int>> connections;
    std::vector<std::string> labels;
    
    for (const auto& transition : transitions) {
        // Find the indices of the source and target states
        int sourceIdx = -1, targetIdx = -1;
        for (size_t i = 0; i < states.size(); ++i) {
            if (states[i].type == transition.from.type) sourceIdx = i;
            if (states[i].type == transition.to.type) targetIdx = i;
        }
        
        if (sourceIdx >= 0 && targetIdx >= 0) {
            connections.emplace_back(sourceIdx, targetIdx);
            labels.push_back(transition.condition);
        }
    }
    
    createEdges(parserNodes, parserEdges, connections, labels);
}

void AutomataVisualizer::layoutNodes(std::vector<Node>& nodes) {
    if (nodes.empty()) return;
    
    // Arrange nodes in a circle
    float radius = std::min(WINDOW_WIDTH, WINDOW_HEIGHT) * 0.35f;
    float centerX = WINDOW_WIDTH / 2.0f;
    float centerY = WINDOW_HEIGHT / 2.0f;
    
    for (size_t i = 0; i < nodes.size(); ++i) {
        float angle = 2.0f * M_PI * i / nodes.size();
        float x = centerX + radius * cos(angle);
        float y = centerY + radius * sin(angle);
        
        nodes[i].shape.setPosition(x, y);
        nodes[i].label.setPosition(x, y);
    }
}

void AutomataVisualizer::createEdges(const std::vector<Node>& nodes, 
                                    std::vector<Edge>& edges, 
                                    const std::vector<std::pair<int, int>>& connections,
                                    const std::vector<std::string>& labels) {
    edges.clear();
    
    for (size_t i = 0; i < connections.size(); ++i) {
        int sourceIdx = connections[i].first;
        int targetIdx = connections[i].second;
        
        if (sourceIdx < 0 || static_cast<size_t>(sourceIdx) >= nodes.size() || 
            targetIdx < 0 || static_cast<size_t>(targetIdx) >= nodes.size()) {
            continue;
        }
        
        Edge edge;
        
        // Create the line between nodes
        edge.line = sf::VertexArray(sf::Lines, 2);
        sf::Vector2f sourcePos = nodes[sourceIdx].shape.getPosition();
        sf::Vector2f targetPos = nodes[targetIdx].shape.getPosition();
        
        // Calculate the vector between nodes
        sf::Vector2f direction = targetPos - sourcePos;
        float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
        sf::Vector2f unit = direction / length;
        
        // Set the position of the line endpoints
        // Start from the edge of the source node and end at the edge of the target node
        sf::Vector2f start = sourcePos + unit * NODE_RADIUS;
        sf::Vector2f end = targetPos - unit * NODE_RADIUS;
        
        edge.line[0].position = start;
        edge.line[0].color = EDGE_COLOR;
        edge.line[1].position = end;
        edge.line[1].color = EDGE_COLOR;
        
        // Create the arrow
        edge.arrow.setRadius(5.0f);
        edge.arrow.setOrigin(5.0f, 5.0f);
        edge.arrow.setFillColor(EDGE_COLOR);
        edge.arrow.setPosition(end);
        
        // Create the edge label
        edge.label.setFont(font);
        edge.label.setString(labels[i]);
        edge.label.setCharacterSize(14);
        edge.label.setFillColor(EDGE_COLOR);
        
        // Position the label in the middle of the edge
        sf::Vector2f midpoint = (start + end) / 2.0f;
        sf::FloatRect textBounds = edge.label.getLocalBounds();
        edge.label.setOrigin(textBounds.width / 2.0f, textBounds.height / 2.0f);
        edge.label.setPosition(midpoint);
        
        edges.push_back(edge);
    }
}

void AutomataVisualizer::handleEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            window.close();
        }
        else if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Escape) {
                window.close();
            }
            else if (event.key.code == sf::Keyboard::Space) {
                switchMode();
            }
        }
    }
}

void AutomataVisualizer::switchMode() {
    currentMode = (currentMode == LEXER) ? PARSER : LEXER;
}

void AutomataVisualizer::drawAutomaton() {
    window.clear(BACKGROUND_COLOR);
    
    // Draw the appropriate automaton based on the current mode
    const auto& nodes = (currentMode == LEXER) ? lexerNodes : parserNodes;
    const auto& edges = (currentMode == LEXER) ? lexerEdges : parserEdges;
    
    // Draw all edges first (so they appear behind nodes)
    for (const auto& edge : edges) {
        window.draw(edge.line);
        window.draw(edge.arrow);
        window.draw(edge.label);
    }
    
    // Draw all nodes
    for (const auto& node : nodes) {
        window.draw(node.shape);
        window.draw(node.label);
    }
    
    // Draw the mode indicator
    sf::Text modeText;
    modeText.setFont(font);
    modeText.setString((currentMode == LEXER) ? "Lexer Automaton (Space to switch)" : "Parser Automaton (Space to switch)");
    modeText.setCharacterSize(20);
    modeText.setFillColor(TEXT_COLOR);
    modeText.setPosition(20.0f, 20.0f);
    window.draw(modeText);
    
    window.display();
}

void AutomataVisualizer::run() {
    while (window.isOpen()) {
        handleEvents();
        drawAutomaton();
    }
}
