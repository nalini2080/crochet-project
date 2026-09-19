
---

## Recommendation Algorithm

Recommendation happens in two distinct phases:

### 1. Hard filtering
Patterns that cannot satisfy the user's request are removed outright, not down-weighted:
- Wrong project category (e.g. user wants HomeDecor, pattern is Amigurumi)
- Difficulty harder than what the user requested

### 2. Weighted scoring
Every remaining pattern is scored per-dimension (style match, time match), each multiplied by a user-adjustable weight and normalized: