#define _USE_MATH_DEFINES
#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>
#include <algorithm>
#include <random>
#include <iostream>

using namespace std;

class Vector2 {
public:
    float x, y;

    Vector2(float _x, float _y) : x(_x), y(_y) {}

    float dot(const Vector2& other) const {
        return x * other.x + y * other.y;
    }
};

// Fisher-Yates �˰���.
void Shuffle(std::vector<int>& arrayToShuffle) {
    std::random_device rd;
    std::mt19937 g(rd());

    for (int e = arrayToShuffle.size() - 1; e > 0; e--) {
        std::uniform_int_distribution<int> distribution(0, e - 1);
        int index = distribution(g);

        std::swap(arrayToShuffle[e], arrayToShuffle[index]);
    }
}

// �̸� ���� ������ ����Ѵ�. 0 - 
std::vector<int> MakePermutation() {
    std::vector<int> permutation;
    for (int i = 0; i < 256; i++) {
        permutation.push_back(i);
    }

    Shuffle(permutation);

    for (int i = 0; i < 256; i++) {
        permutation.push_back(permutation[i]);
    }

    return permutation;
}

const std::vector<int> Permutation = MakePermutation();

Vector2 GetGradientVector(int v) {
    int h = v & 3;
    if (h == 0)
        return Vector2(1.0f, 1.0f);
    else if (h == 1)
        return Vector2(-1.0f, 1.0f);
    else if (h == 2)
        return Vector2(-1.0f, -1.0f);
    else
        return Vector2(1.0f, -1.0f);
}

float Fade(float t) {
    return ((6 * t - 15) * t + 10) * t * t * t;
}

float Lerp(float t, float a1, float a2) {
    return a1 + t * (a2 - a1);
}

float Noise2D(float x, float y) {
    // 1. �� x, y �Ǽ��� �Է����� �ξ��� ��, �ȼ����� ���� �׸��忡 �����մϴ�.

    int X = static_cast<int>(std::floor(x)) & 255;
    int Y = static_cast<int>(std::floor(y)) & 255;

    // 1. �Ҽ��θ� ����� �մϴ�.
    float xf = x - std::floor(x);
    float yf = y - std::floor(y);

    // 2. �ش� ���簢�� �׸��� �� �������� �׷����Ʈ(����) ���͸� �Ҵ��մϴ�.
    // 2-1. Ken Perlin �� ������ ������ �����մϴ�.
    // 2-2. �������� ���� �����ɴϴ�. �� �𼭸��鿡�� ������ ���� �Ҵ��մϴ�.
    int valueTopRight = Permutation[Permutation[X + 1] + Y + 1];
    int valueTopLeft = Permutation[Permutation[X] + Y + 1];
    int valueBottomRight = Permutation[Permutation[X + 1] + Y];
    int valueBottomLeft = Permutation[Permutation[X] + Y];

    // 3. �� �𼭸������� �Է� �������� �Ÿ� ���͸� ���մϴ�.
    Vector2 topRight(xf - 1.0f, yf - 1.0f);
    Vector2 topLeft(xf, yf - 1.0f);
    Vector2 bottomRight(xf - 1.0f, yf);
    Vector2 bottomLeft(xf, yf);

    // 4. �� �𼭸� ���� ������ ����(�Է� �������� �Ÿ� ����) �� �𼭸� ���� ���⸦ �����մϴ�. �׷����Ʈ ������ ������� �Ÿ��� ���� Ŀ���ϴ�.
    float dotTopRight = topRight.dot(GetGradientVector(valueTopRight));
    float dotTopLeft = topLeft.dot(GetGradientVector(valueTopLeft));
    float dotBottomRight = bottomRight.dot(GetGradientVector(valueBottomRight));
    float dotBottomLeft = bottomLeft.dot(GetGradientVector(valueBottomLeft));

    // 5. �ε巯�� ����(smooth interpolation)�� ���� ���˴ϴ�. 
    // �� �Լ��� �Է� �� t�� 0���� 1 ������ ������ �����ϸ�, ���׽� ��� ���� ��ȭ�մϴ�.
    float u = Fade(xf);
    float v = Fade(yf);

    // 6. ������ ������ ȥ���Ͽ� ������ ������ ����ϴ�. 
    float result = Lerp(v,
        Lerp(u, dotBottomLeft, dotBottomRight), Lerp(u, dotTopLeft, dotTopRight));

    return result;
}

float FractalBrownianMotion(float x, float y, int numOctaves) {
    float result = 0.0f;
    float amplitude = 1.0f;
    float frequency = 0.005f;
    float persistence = 0.5f;

    for (int octave = 0; octave < numOctaves; octave++) {
        float n = amplitude * Noise2D(x * frequency, y * frequency);
        result += n;

        amplitude *= persistence; 
        frequency *= 2.0f;
    }

    result = std::max(-1.0f, std::min(1.0f, result));

    return result;
}

int main() {
    const int windowWidth = 800;
    const int windowHeight = 600;
    const int numOctaves = 8;

    sf::RenderWindow window(sf::VideoMode(windowWidth, windowHeight), "Perlin Noise");

    sf::Image textureImage;
    textureImage.create(windowWidth, windowHeight);

    for (int y = 0; y < windowHeight; y++) {
        for (int x = 0; x < windowWidth; x++) {
            float persistence = 0.5f;
            float frequency = 0.01f;
            //float n = Noise2D(x * frequency, y * frequency);
            //float value = n;
            float value = FractalBrownianMotion(static_cast<float>(x), static_cast<float>(y), numOctaves);
            value = (value + 1.0f) / 2.0f; // Normalize to [0, 1]

            sf::Color color(static_cast<sf::Uint8>(255 * value), static_cast<sf::Uint8>(255 * value), static_cast<sf::Uint8>(255 * value));
            textureImage.setPixel(x, y, color);
        }
    }

    sf::Texture texture;
    texture.loadFromImage(textureImage);
    sf::Sprite sprite(texture);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear();
        window.draw(sprite);
        window.display();
    }

    return 0;
}
