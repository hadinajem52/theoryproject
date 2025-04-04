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
    
    // Method to process Python code
    void processPythonCode(const std::string& code);
    
    // Method to load and process example.py
    void loadAndProcessExampleFile();
    
private:
    sf::RenderWindow window;
    sf::Font font;
    
    // Automaton visualization data
    struct Node {
        sf::CircleShape shape;
        sf::Text label;
        bool isActive;
        bool isAccepting;
        bool isInitial;
        
        Node() : isActive(false), isAccepting(false), isInitial(false) {}
    };
    
    struct Edge {
        sf::VertexArray line;
        sf::Text label;
        sf::ConvexShape arrow;  // Changed from CircleShape to ConvexShape for triangular arrows
        bool isSelfLoop;
        
        Edge() : isSelfLoop(false) {}
    };
    
    std::vector<Node> lexerNodes;
    std::vector<Edge> lexerEdges;
    std::vector<Node> parserNodes;
    std::vector<Edge> parserEdges;
    
    // Cache for states and transitions
    std::vector<Lexer::State> lexerStates;
    std::vector<Lexer::Transition> lexerTransitions;
    std::vector<Parser::State> parserStates;
    std::vector<Parser::Transition> parserTransitions;
    
    // Code processing data
    std::string currentCode;
    std::vector<Lexer::TraceStep> lexerTrace;
    std::vector<Parser::TraceStep> parserTrace;
    
    // UI for code input
    sf::RectangleShape codeInputBox;
    sf::Text codeInputText;
    sf::Text codeInputPrompt;
    sf::RectangleShape runButton;
    sf::Text runButtonText;
    bool isEditingCode = false;
    std::string inputBuffer;
    
    // UI for example file loading
    sf::RectangleShape loadExampleButton;
    sf::Text loadExampleText;
    
    // Helper method to read a file
    std::string readFile(const std::string& filename);
    
    // For node dragging
    Node* selectedNode = nullptr;
    
    // Layout options
    enum LayoutType { CIRCULAR, FORCE_DIRECTED };
    LayoutType currentLayout = CIRCULAR;
    
    // Current visualization mode
    enum Mode { LEXER, PARSER } currentMode;
    
    // Animation data
    bool isAnimating = false;
    float animationSpeed = 1.0f; // seconds per transition
    sf::Clock animationClock;
    std::vector<std::pair<int, int>> animationSequence; // pairs of (fromStateIdx, toStateIdx)
    std::vector<std::string> animationInput; // tokens or characters processed
    size_t currentAnimationStep = 0;
    std::string currentInput; // Currently processed input for display
    
    // Helper methods
    void drawAutomaton();
    void layoutNodes(std::vector<Node>& nodes);
    void applyForceDirectedLayout(std::vector<Node>& nodes, int iterations = 50);
    void createEdges(const std::vector<Node>& nodes, 
                   std::vector<Edge>& edges, 
                   const std::vector<std::pair<int, int>>& connections,
                   const std::vector<std::string>& labels);
    void handleEvents();
    void switchMode();
    void switchLayout();
    void updateEdges();
    
    // Methods for animation
    void startAnimation();
    void updateAnimation();
    void animateTransition(int fromIdx, int toIdx, const std::string& input);
    void loadSimulationData();
    void loadRealSimulationData();
    
    // Methods for code input UI
    void drawCodeInputUI();
    void handleCodeInputEvents(const sf::Event& event);
};
