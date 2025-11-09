#!/usr/bin/env python3
"""
Simple HackDS Example Game
A basic bouncing ball demo
"""

import pygame
import sys
import random

# Initialize Pygame
pygame.init()

# Screen settings
SCREEN_WIDTH = 1280
SCREEN_HEIGHT = 720
FPS = 60

# Colors
BLACK = (0, 0, 0)
WHITE = (255, 255, 255)
RED = (255, 0, 0)
GREEN = (0, 255, 0)
BLUE = (0, 100, 255)

class Ball:
    def __init__(self, x, y, radius, color):
        self.x = x
        self.y = y
        self.radius = radius
        self.color = color
        self.vx = random.randint(-5, 5)
        self.vy = random.randint(-5, 5)
        if self.vx == 0:
            self.vx = 3
        if self.vy == 0:
            self.vy = 3

    def update(self):
        self.x += self.vx
        self.y += self.vy

        # Bounce off walls
        if self.x - self.radius <= 0 or self.x + self.radius >= SCREEN_WIDTH:
            self.vx = -self.vx
        if self.y - self.radius <= 0 or self.y + self.radius >= SCREEN_HEIGHT:
            self.vy = -self.vy

    def draw(self, screen):
        pygame.draw.circle(screen, self.color, (int(self.x), int(self.y)), self.radius)


def main():
    # Create screen
    screen = pygame.display.set_mode((SCREEN_WIDTH, SCREEN_HEIGHT))
    pygame.display.set_caption("HackDS Example Game")
    clock = pygame.time.Clock()

    # Create balls
    balls = [
        Ball(640, 360, 30, RED),
        Ball(400, 200, 25, GREEN),
        Ball(800, 500, 20, BLUE),
    ]

    # Font
    font = pygame.font.Font(None, 36)

    running = True
    while running:
        # Handle events
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_ESCAPE or event.key == pygame.K_q:
                    running = False
                elif event.key == pygame.K_SPACE:
                    # Add a new ball
                    x = random.randint(50, SCREEN_WIDTH - 50)
                    y = random.randint(50, SCREEN_HEIGHT - 50)
                    radius = random.randint(15, 40)
                    color = (
                        random.randint(50, 255),
                        random.randint(50, 255),
                        random.randint(50, 255)
                    )
                    balls.append(Ball(x, y, radius, color))

        # Update
        for ball in balls:
            ball.update()

        # Draw
        screen.fill(BLACK)

        for ball in balls:
            ball.draw(screen)

        # Draw instructions
        text = font.render("SPACE: Add Ball | ESC: Exit", True, WHITE)
        screen.blit(text, (20, 20))

        ball_count = font.render(f"Balls: {len(balls)}", True, WHITE)
        screen.blit(ball_count, (SCREEN_WIDTH - 150, 20))

        pygame.display.flip()
        clock.tick(FPS)

    pygame.quit()


if __name__ == '__main__':
    main()
