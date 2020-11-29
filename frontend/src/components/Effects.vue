<template>
  <v-card class="mx-auto" max-width="400">
    <v-card-title>Effects selection</v-card-title>
    <v-card-text>
      <v-skeleton-loader v-if="loading" type="list-item-avatar-three-line"></v-skeleton-loader>
      <v-radio-group v-if="!loading" v-bind:value="currentEffect" @change="onChange">
        <v-radio v-for="n in effects" :key="n" :label="`${n}`" :value="n"></v-radio>
      </v-radio-group>
    </v-card-text>
  </v-card>
</template>

<script>
export default {
  name: "Effects",
  props: {
    loading: Boolean,
    effects: Array,
    currentEffect: String,
  },
  methods: {
    onChange(value) {
      this.$emit('switch-effect', value)
    }
  },
  data() {
    return {
      effect: ''
    };
  }
};
</script>
