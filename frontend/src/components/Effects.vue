<template>
  <v-card class="mx-auto" max-width="400">
    <v-card-title>Effects selection</v-card-title>
    <v-card-text>
      <v-radio-group v-model="effects">
        <v-radio v-for="n in 8" :key="n" :label="`Radio ${n}`" :value="n"></v-radio>
      </v-radio-group>
    </v-card-text>
  </v-card>
</template>

<script>
export default {
  name: "Effects",
  data() {
    return {
      effects: 1
    };
  }
};
</script>
