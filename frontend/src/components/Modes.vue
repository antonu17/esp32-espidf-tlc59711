<template>
  <v-card class="mx-auto" max-width="400">
    <v-card-title>Mode selection</v-card-title>
    <v-card-text>
      <v-skeleton-loader v-if="loading" type="list-item-avatar-three-line"></v-skeleton-loader>
      <v-radio-group v-if="!loading" v-bind:value="currentMode" @change="onChange">
        <v-radio v-for="n in modes" :key="n.id" :label="`${n.name}`" :value="n.id"></v-radio>
      </v-radio-group>
    </v-card-text>
  </v-card>
</template>

<script>
export default {
  name: "Modes",
  props: {
    loading: Boolean,
    currentMode: Number
  },
  methods: {
    onChange(value) {
      this.$emit("switch-mode", value);
    }
  },
  data() {
    return {
      modes: [
        {
          id: 0,
          name: "Loop"
        },
        {
          id: 1,
          name: "Solo"
        }
      ]
    };
  }
};
</script>
