<template>
  <v-container>
    <Loader v-if="is_loading" />
    <Effects v-if="!is_loading" v-bind:effects="effects"/>
  </v-container>
</template>

<script>
// @ is an alias to /src
import Effects from "@/components/Effects.vue";
import Loader from "@/components/Loader.vue";

export default {
  name: "Home",
  components: {
    Effects, Loader
  },
  computed: {
    is_loading() {
      return this.$store.state.loadingStatus === 'loading';
    },
    effects() {
      return this.$store.state.effects;
    }
  },
  mounted() {
    this.$store.dispatch('fetchEffects');
  }
};
</script>
