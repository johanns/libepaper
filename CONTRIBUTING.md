# Contributing to libepaper

We welcome contributions! This project uses **Conventional Commits** for clear history.

## Commit Format

```
<type>(<scope>): <subject>

<body>

<footer>
```

**Types:**
- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation changes
- `style`: Code formatting
- `refactor`: Code refactoring
- `perf`: Performance improvements
- `test`: Test additions/updates
- `chore`: Maintenance tasks

**Scopes:** `device`, `driver`, `display`, `docs`, `examples`, `tests`

### Examples

```
feat(driver): add EPD42 display driver support

Implements the Driver interface for Waveshare 4.2" displays.
Supports both black/white and 4-level grayscale modes.

fix(display): correct auto-wake behavior in refresh()

The previous implementation didn't check is_asleep_ before refresh.
Now transparently wakes display if needed.

docs(readme): restructure documentation for clarity
```

## Pull Request Process

1. Fork and create a feature branch
2. Follow project coding standards (run `./bin/format`)
3. Add/update tests as needed
4. Use conventional commit messages
5. Update documentation
6. Submit PR with clear description
