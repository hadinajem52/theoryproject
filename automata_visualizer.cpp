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
const sf::Color ACCEPTING_COLOR(220, 220, 100);  // Yellow-ish for accepting states
const sf::Color INITIAL_COLOR(100, 220, 220);    // Cyan-ish for initial states
const sf::Color TEXT_COLOR(255, 255, 255);
const sf::Color EDGE_COLOR(200, 200, 200);
const sf::Color ERROR_EDGE_COLOR(255, 100, 100); // Red for error transitions
const sf::Color EPSILON_EDGE_COLOR(100, 100, 255); // Blue for epsilon transitions
const sf::Color BACKGROUND_COLOR(50, 50, 50);

AutomataVisualizer::AutomataVisualizer() : currentMode(LEXER), currentLayout(CIRCULAR), selectedNode(nullptr) {
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
    // Cache states and transitions for future updates
    lexerStates = lexer.getStates();
    lexerTransitions = lexer.getTransitions();
    auto currentState = lexer.getCurrentState();
    
    // Clear existing nodes and edges
    lexerNodes.clear();
    lexerEdges.clear();
    
    // Create nodes for each state
    for (const auto& state : lexerStates) {
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
        
        // Set accepting and initial states based on state type
        node.isAccepting = (state.type == Lexer::State::STRING || 
                           state.type == Lexer::State::NUMBER || 
                           state.type == Lexer::State::IDENTIFIER);
        node.isInitial = (state.type == Lexer::State::START);
        
        lexerNodes.push_back(node);
    }
    
    // Apply the current layout
    if (currentLayout == CIRCULAR) {
        layoutNodes(lexerNodes);
    } else {
        applyForceDirectedLayout(lexerNodes);
    }
    
    // Create edges for transitions
    std::vector<std::pair<int, int>> connections;
    std::vector<std::string> labels;
    
    for (const auto& transition : lexerTransitions) {
        // Find the indices of the source and target states
        int sourceIdx = -1, targetIdx = -1;
        for (size_t i = 0; i < lexerStates.size(); ++i) {
            if (lexerStates[i].type == transition.from.type) sourceIdx = i;
            if (lexerStates[i].type == transition.to.type) targetIdx = i;
        }
        
        if (sourceIdx >= 0 && targetIdx >= 0) {
            connections.emplace_back(sourceIdx, targetIdx);
            labels.push_back(transition.condition);
        }
    }
    
    createEdges(lexerNodes, lexerEdges, connections, labels);
}

void AutomataVisualizer::visualizeParser(const Parser& parser) {
    // Cache states and transitions for future updates
    parserStates = parser.getStates();
    parserTransitions = parser.getTransitions();
    auto currentState = parser.getCurrentState();
    
    // Clear existing nodes and edges
    parserNodes.clear();
    parserEdges.clear();
    
    // Create nodes for each state
    for (const auto& state : parserStates) {
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
        
        // Set accepting and initial states based on state type
        node.isAccepting = (state.type == Parser::EXPRESSION || 
                           state.type == Parser::STATEMENT);
        node.isInitial = (state.type == Parser::PROGRAM);
        
        parserNodes.push_back(node);
    }
    
    // Apply the current layout
    if (currentLayout == CIRCULAR) {
        layoutNodes(parserNodes);
    } else {
        applyForceDirectedLayout(parserNodes);
    }
    
    // Create edges for transitions
    std::vector<std::pair<int, int>> connections;
    std::vector<std::string> labels;
    
    for (const auto& transition : parserTransitions) {
        // Find the indices of the source and target states
        int sourceIdx = -1, targetIdx = -1;
        for (size_t i = 0; i < parserStates.size(); ++i) {
            if (parserStates[i].type == transition.from.type) sourceIdx = i;
            if (parserStates[i].type == transition.to.type) targetIdx = i;
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
        edge.isSelfLoop = (sourceIdx == targetIdx);
        sf::Vector2f sourcePos = nodes[sourceIdx].shape.getPosition();
        sf::Vector2f targetPos = nodes[targetIdx].shape.getPosition();
        
        sf::Color transitionColor = EDGE_COLOR;
        if (labels[i].find("error") != std::string::npos) {
            transitionColor = ERROR_EDGE_COLOR;
        } else if (labels[i].find("epsilon") != std::string::npos || 
                   labels[i].find("ε") != std::string::npos) {
            transitionColor = EPSILON_EDGE_COLOR;
        }
        
        if (edge.isSelfLoop) {
            edge.line = sf::VertexArray(sf::LinesStrip, 30);
            
            sf::Vector2f center = sourcePos;
            float radius = NODE_RADIUS * 1.5f;
            float startAngle = -M_PI/4;
            float endAngle = -3*M_PI/4;
            
            for (int j = 0; j < 30; j++) {
                float angle = startAngle + (endAngle - startAngle) * j / 29.0f;
                sf::Vector2f point(
                    center.x + std::cos(angle) * radius,
                    center.y + std::sin(angle) * radius
                );
                edge.line[j].position = point;
                edge.line[j].color = transitionColor;
            }
            
            edge.label.setFont(font);
            edge.label.setString(labels[i]);
            edge.label.setCharacterSize(14);
            edge.label.setFillColor(transitionColor);
            
            sf::Vector2f labelPos(center.x, center.y - radius - 10);
            sf::FloatRect textBounds = edge.label.getLocalBounds();
            edge.label.setOrigin(textBounds.width / 2.0f, textBounds.height / 2.0f);
            edge.label.setPosition(labelPos);
            
            edge.arrow.setPointCount(3);
            float arrowSize = 8.0f;
            sf::Vector2f arrowPos = edge.line[22].position;
            float arrowAngle = endAngle + M_PI/2;
            
            edge.arrow.setPoint(0, sf::Vector2f(0, 0));
            edge.arrow.setPoint(1, sf::Vector2f(-arrowSize, arrowSize));
            edge.arrow.setPoint(2, sf::Vector2f(-arrowSize, -arrowSize));
            
            edge.arrow.setFillColor(transitionColor);
            edge.arrow.setOrigin(0, 0);
            edge.arrow.setPosition(arrowPos);
            edge.arrow.setRotation(arrowAngle * 180/M_PI);
        } else {
            edge.line = sf::VertexArray(sf::Lines, 2);
            
            sf::Vector2f direction = targetPos - sourcePos;
            float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
            sf::Vector2f unit = direction / length;
            
            sf::Vector2f start = sourcePos + unit * NODE_RADIUS;
            sf::Vector2f end = targetPos - unit * NODE_RADIUS;
            
            edge.line[0].position = start;
            edge.line[0].color = transitionColor;
            edge.line[1].position = end;
            edge.line[1].color = transitionColor;
            
            edge.arrow.setPointCount(3);
            float arrowSize = 8.0f;
            float arrowAngle = std::atan2(direction.y, direction.x) * 180 / M_PI;
            
            edge.arrow.setPoint(0, sf::Vector2f(0, 0));
            edge.arrow.setPoint(1, sf::Vector2f(-arrowSize, arrowSize/2));
            edge.arrow.setPoint(2, sf::Vector2f(-arrowSize, -arrowSize/2));
            
            edge.arrow.setFillColor(transitionColor);
            edge.arrow.setOrigin(0, 0);
            edge.arrow.setPosition(end);
            edge.arrow.setRotation(arrowAngle);
            
            edge.label.setFont(font);
            edge.label.setString(labels[i]);
            edge.label.setCharacterSize(14);
            edge.label.setFillColor(transitionColor);
            
            sf::Vector2f midpoint = (start + end) / 2.0f;
            sf::Vector2f normal(-unit.y, unit.x);
            sf::Vector2f offset = normal * 10.0f;
            sf::FloatRect textBounds = edge.label.getLocalBounds();
            edge.label.setOrigin(textBounds.width / 2.0f, textBounds.height / 2.0f);
            edge.label.setPosition(midpoint + offset);
        }
        
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
            else if (event.key.code == sf::Keyboard::L) {
                switchLayout();
            }
        }
        else if (event.type == sf::Event::MouseButtonPressed) {
            if (event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2f mousePos(event.mouseButton.x, event.mouseButton.y);
                auto& nodes = (currentMode == LEXER) ? lexerNodes : parserNodes;
                for (auto& node : nodes) {
                    sf::Vector2f nodePos = node.shape.getPosition();
                    float dx = mousePos.x - nodePos.x;
                    float dy = mousePos.y - nodePos.y;
                    if (sqrt(dx*dx + dy*dy) < NODE_RADIUS) {
                        selectedNode = &node;
                        break;
                    }
                }
            }
        }
        else if (event.type == sf::Event::MouseButtonReleased) {
            selectedNode = nullptr;
        }
        else if (event.type == sf::Event::MouseMoved) {
            if (selectedNode) {
                selectedNode->shape.setPosition(event.mouseMove.x, event.mouseMove.y);
                selectedNode->label.setPosition(event.mouseMove.x, event.mouseMove.y);
                updateEdges();
            }
        }
    }
}

void AutomataVisualizer::switchLayout() {
    currentLayout = (currentLayout == CIRCULAR) ? FORCE_DIRECTED : CIRCULAR;
    
    // Apply the new layout to current nodes
    auto& nodes = (currentMode == LEXER) ? lexerNodes : parserNodes;
    
    if (currentLayout == CIRCULAR) {
        layoutNodes(nodes);
    } else {
        applyForceDirectedLayout(nodes);
    }
    
    // Update edges after changing layout
    updateEdges();
}

void AutomataVisualizer::updateEdges() {
    if (currentMode == LEXER) {
        // Handle lexer mode
        std::vector<std::pair<int, int>> connections;
        std::vector<std::string> labels;
        
        for (const auto& transition : lexerTransitions) {
            int sourceIdx = -1, targetIdx = -1;
            for (size_t i = 0; i < lexerStates.size(); ++i) {
                if (lexerStates[i].type == transition.from.type) sourceIdx = i;
                if (lexerStates[i].type == transition.to.type) targetIdx = i;
            }
            
            if (sourceIdx >= 0 && targetIdx >= 0) {
                connections.emplace_back(sourceIdx, targetIdx);
                labels.push_back(transition.condition);
            }
        }
        
        createEdges(lexerNodes, lexerEdges, connections, labels);
    } else {
        // Handle parser mode
        std::vector<std::pair<int, int>> connections;
        std::vector<std::string> labels;
        
        for (const auto& transition : parserTransitions) {
            int sourceIdx = -1, targetIdx = -1;
            for (size_t i = 0; i < parserStates.size(); ++i) {
                if (parserStates[i].type == transition.from.type) sourceIdx = i;
                if (parserStates[i].type == transition.to.type) targetIdx = i;
            }
            
            if (sourceIdx >= 0 && targetIdx >= 0) {
                connections.emplace_back(sourceIdx, targetIdx);
                labels.push_back(transition.condition);
            }
        }
        
        createEdges(parserNodes, parserEdges, connections, labels);
    }
}

void AutomataVisualizer::applyForceDirectedLayout(std::vector<Node>& nodes, int iterations) {
    if (nodes.size() <= 1) return;
    
    // Constants for the force-directed algorithm
    const float k = 0.05f;  // Spring constant
    const float repulsion = 10000.0f;  // Repulsion constant
    const float damping = 0.95f;  // Damping factor
    
    // Initialize velocities
    std::vector<sf::Vector2f> velocities(nodes.size(), sf::Vector2f(0, 0));
    
    // Create a list of edges for the algorithm
    std::vector<std::pair<int, int>> edgeList;
    
    if (currentMode == LEXER) {
        for (const auto& transition : lexerTransitions) {
            int sourceIdx = -1, targetIdx = -1;
            for (size_t i = 0; i < lexerStates.size(); ++i) {
                if (lexerStates[i].type == transition.from.type) sourceIdx = i;
                if (lexerStates[i].type == transition.to.type) targetIdx = i;
            }
            
            if (sourceIdx >= 0 && targetIdx >= 0) {
                edgeList.emplace_back(sourceIdx, targetIdx);
            }
        }
    } else {
        for (const auto& transition : parserTransitions) {
            int sourceIdx = -1, targetIdx = -1;
            for (size_t i = 0; i < parserStates.size(); ++i) {
                if (parserStates[i].type == transition.from.type) sourceIdx = i;
                if (parserStates[i].type == transition.to.type) targetIdx = i;
            }
            
            if (sourceIdx >= 0 && targetIdx >= 0) {
                edgeList.emplace_back(sourceIdx, targetIdx);
            }
        }
    }
    
    // Start with a circular layout as initial positions
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
    
    // Main force-directed algorithm iterations
    for (int iter = 0; iter < iterations; ++iter) {
        // Calculate repulsive forces
        for (size_t i = 0; i < nodes.size(); ++i) {
            sf::Vector2f pos1 = nodes[i].shape.getPosition();
            
            for (size_t j = 0; j < nodes.size(); ++j) {
                if (i == j) continue;
                
                sf::Vector2f pos2 = nodes[j].shape.getPosition();
                sf::Vector2f direction = pos1 - pos2;
                float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);
                
                // Avoid division by zero
                if (distance < 1.0f) distance = 1.0f;
                
                // Repulsive force inversely proportional to distance
                float force = repulsion / (distance * distance);
                
                // Normalize direction vector
                direction /= distance;
                
                // Apply force to velocity
                velocities[i] += direction * force;
            }
        }
        
        // Calculate attractive forces (springs)
        for (const auto& edge : edgeList) {
            sf::Vector2f pos1 = nodes[edge.first].shape.getPosition();
            sf::Vector2f pos2 = nodes[edge.second].shape.getPosition();
            sf::Vector2f direction = pos2 - pos1;
            float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);
            
            // Avoid division by zero
            if (distance < 1.0f) distance = 1.0f;
            
            // Spring force proportional to distance
            float force = k * distance;
            
            // Normalize direction vector
            direction /= distance;
            
            // Apply force to velocities
            velocities[edge.first] += direction * force;
            velocities[edge.second] -= direction * force;
        }
        
        // Apply velocities to positions with damping
        for (size_t i = 0; i < nodes.size(); ++i) {
            velocities[i] *= damping;
            nodes[i].shape.move(velocities[i]);
            nodes[i].label.setPosition(nodes[i].shape.getPosition());
        }
    }
    
    // Keep nodes within window bounds
    for (auto& node : nodes) {
        sf::Vector2f pos = node.shape.getPosition();
        pos.x = std::max(NODE_RADIUS + 20.0f, std::min(pos.x, WINDOW_WIDTH - NODE_RADIUS - 20.0f));
        pos.y = std::max(NODE_RADIUS + 20.0f, std::min(pos.y, WINDOW_HEIGHT - NODE_RADIUS - 20.0f));
        node.shape.setPosition(pos);
        node.label.setPosition(pos);
    }
}

void AutomataVisualizer::switchMode() {
    currentMode = (currentMode == LEXER) ? PARSER : LEXER;
}

void AutomataVisualizer::drawAutomaton() {
    window.clear(BACKGROUND_COLOR);
    
    const auto& nodes = (currentMode == LEXER) ? lexerNodes : parserNodes;
    const auto& edges = (currentMode == LEXER) ? lexerEdges : parserEdges;
    
    for (const auto& edge : edges) {
        window.draw(edge.line);
        window.draw(edge.arrow);
        window.draw(edge.label);
    }
    
    for (const auto& node : nodes) {
        sf::CircleShape nodeShape = node.shape;
        if (node.isActive && node.isAccepting) {
            nodeShape.setFillColor(sf::Color(150, 255, 150));
        } else if (node.isActive) {
            nodeShape.setFillColor(ACTIVE_COLOR);
        } else if (node.isAccepting) {
            nodeShape.setFillColor(ACCEPTING_COLOR);
        } else if (node.isInitial) {
            nodeShape.setFillColor(INITIAL_COLOR);
        } else {
            nodeShape.setFillColor(INACTIVE_COLOR);
        }
        window.draw(nodeShape);
        
        if (node.isAccepting) {
            sf::CircleShape outerCircle;
            outerCircle.setRadius(NODE_RADIUS - 4);
            outerCircle.setOutlineThickness(2);
            outerCircle.setOutlineColor(nodeShape.getFillColor());
            outerCircle.setFillColor(sf::Color::Transparent);
            outerCircle.setOrigin(NODE_RADIUS - 4, NODE_RADIUS - 4);
            outerCircle.setPosition(node.shape.getPosition());
            window.draw(outerCircle);
        }
        
        if (node.isInitial) {
            sf::VertexArray line(sf::Lines, 2);
            sf::Vector2f nodePos = node.shape.getPosition();
            line[0].position = sf::Vector2f(nodePos.x - NODE_RADIUS - 30, nodePos.y);
            line[1].position = sf::Vector2f(nodePos.x - NODE_RADIUS - 10, nodePos.y);
            line[0].color = TEXT_COLOR;
            line[1].color = TEXT_COLOR;
            window.draw(line);
            
            sf::ConvexShape arrow;
            arrow.setPointCount(3);
            float arrowSize = 12.0f;
            
            arrow.setPoint(0, sf::Vector2f(0, 0));
            arrow.setPoint(1, sf::Vector2f(-arrowSize, arrowSize/2));
            arrow.setPoint(2, sf::Vector2f(-arrowSize, -arrowSize/2));
            
            arrow.setFillColor(TEXT_COLOR);
            
            arrow.setPosition(nodePos.x - NODE_RADIUS - 10, nodePos.y);
            
            window.draw(arrow);
        }
        
        window.draw(node.label);
    }
    
    sf::Text modeText;
    modeText.setFont(font);
    modeText.setString((currentMode == LEXER) ? "Lexer Automaton (Space to switch)" : "Parser Automaton (Space to switch)");
    modeText.setCharacterSize(20);
    modeText.setFillColor(TEXT_COLOR);
    modeText.setPosition(20.0f, 20.0f);
    window.draw(modeText);
    
    // Add layout type indicator
    sf::Text layoutText;
    layoutText.setFont(font);
    layoutText.setString((currentLayout == CIRCULAR) ? "Circular Layout (L to switch)" : "Force-Directed Layout (L to switch)");
    layoutText.setCharacterSize(16);
    layoutText.setFillColor(TEXT_COLOR);
    layoutText.setPosition(20.0f, 50.0f);
    window.draw(layoutText);
    
    window.display();
}

void AutomataVisualizer::run() {
    while (window.isOpen()) {
        handleEvents();
        drawAutomaton();
    }
}
